/*
 * lwf - Wireless Information Library - Command line frontend
 *
 *   Copyright (C) 2011 Jo-Philipp Wich <xm@subsignal.org>
 *
 * The lwf library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * The lwf library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the lwf library. If not, see http://www.gnu.org/licenses/.
 */

#include <stdio.h>
#include <glob.h>

#include "lwf.h"


static char * format_bssid(unsigned char *mac)
{
	static char buf[18];

	snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
		 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	return buf;
}

static char * format_ssid(char *ssid)
{
	static char buf[LWF_ESSID_MAX_SIZE + 3];

	if (ssid && ssid[0])
		snprintf(buf, sizeof(buf), "\"%s\"", ssid);
	else
		snprintf(buf, sizeof(buf), "unknown");

	return buf;
}

static char * format_channel(int ch)
{
	static char buf[8];

	if (ch <= 0)
		snprintf(buf, sizeof(buf), "unknown");
	else
		snprintf(buf, sizeof(buf), "%d", ch);

	return buf;
}

static char * format_frequency(int freq)
{
	static char buf[10];

	if (freq <= 0)
		snprintf(buf, sizeof(buf), "unknown");
	else
		snprintf(buf, sizeof(buf), "%.3f GHz", ((float)freq / 1000.0));

	return buf;
}

static char * format_txpower(int pwr)
{
	static char buf[10];

	if (pwr < 0)
		snprintf(buf, sizeof(buf), "unknown");
	else
		snprintf(buf, sizeof(buf), "%d dBm", pwr);

	return buf;
}

static char * format_quality(int qual)
{
	static char buf[8];

	if (qual < 0)
		snprintf(buf, sizeof(buf), "unknown");
	else
		snprintf(buf, sizeof(buf), "%d", qual);

	return buf;
}

static char * format_quality_max(int qmax)
{
	static char buf[8];

	if (qmax < 0)
		snprintf(buf, sizeof(buf), "unknown");
	else
		snprintf(buf, sizeof(buf), "%d", qmax);

	return buf;
}

static char * format_signal(int sig)
{
	static char buf[10];

	if (!sig)
		snprintf(buf, sizeof(buf), "unknown");
	else
		snprintf(buf, sizeof(buf), "%d dBm", sig);

	return buf;
}

static char * format_noise(int noise)
{
	static char buf[10];

	if (!noise)
		snprintf(buf, sizeof(buf), "unknown");
	else
		snprintf(buf, sizeof(buf), "%d dBm", noise);

	return buf;
}

static char * format_rate(int rate)
{
	static char buf[14];

	if (rate <= 0)
		snprintf(buf, sizeof(buf), "unknown");
	else
		snprintf(buf, sizeof(buf), "%d.%d MBit/s",
			 rate / 1000, (rate % 1000) / 100);

	return buf;
}

static char * format_enc_ciphers(int ciphers)
{
	static char str[128] = { 0 };
	char *pos = str;

	if (ciphers & LWF_CIPHER_WEP40)
		pos += sprintf(pos, "WEP-40, ");

	if (ciphers & LWF_CIPHER_WEP104)
		pos += sprintf(pos, "WEP-104, ");

	if (ciphers & LWF_CIPHER_TKIP)
		pos += sprintf(pos, "TKIP, ");

	if (ciphers & LWF_CIPHER_CCMP)
		pos += sprintf(pos, "CCMP, ");

	if (ciphers & LWF_CIPHER_WRAP)
		pos += sprintf(pos, "WRAP, ");

	if (ciphers & LWF_CIPHER_AESOCB)
		pos += sprintf(pos, "AES-OCB, ");

	if (ciphers & LWF_CIPHER_CKIP)
		pos += sprintf(pos, "CKIP, ");

	if (!ciphers || (ciphers & LWF_CIPHER_NONE))
		pos += sprintf(pos, "NONE, ");

	*(pos - 2) = 0;

	return str;
}

static char * format_enc_suites(int suites)
{
	static char str[64] = { 0 };
	char *pos = str;

	if (suites & LWF_KMGMT_PSK)
		pos += sprintf(pos, "PSK/");

	if (suites & LWF_KMGMT_8021x)
		pos += sprintf(pos, "802.1X/");

	if (!suites || (suites & LWF_KMGMT_NONE))
		pos += sprintf(pos, "NONE/");

	*(pos - 1) = 0;

	return str;
}

static char * format_encryption(struct lwf_crypto_entry *c)
{
	static char buf[512];

	if (!c) {
		snprintf(buf, sizeof(buf), "unknown");
	} else if (c->enabled) {
		/* WEP */
		if (c->auth_algs && !c->wpa_version) {
			if ((c->auth_algs & LWF_AUTH_OPEN) &&
			    (c->auth_algs & LWF_AUTH_SHARED))
				snprintf(buf, sizeof(buf), "WEP Open/Shared (%s)",
					 format_enc_ciphers(c->pair_ciphers));
			else if (c->auth_algs & LWF_AUTH_OPEN)
				snprintf(buf, sizeof(buf), "WEP Open System (%s)",
					 format_enc_ciphers(c->pair_ciphers));
			else if (c->auth_algs & LWF_AUTH_SHARED)
				snprintf(buf, sizeof(buf), "WEP Shared Auth (%s)",
					 format_enc_ciphers(c->pair_ciphers));
		}
		/* WPA */
		else if (c->wpa_version) {
			switch (c->wpa_version) {
			case 3:
				snprintf(buf, sizeof(buf), "mixed WPA/WPA2 %s (%s)",
					 format_enc_suites(c->auth_suites),
					 format_enc_ciphers(c->pair_ciphers | c->group_ciphers));
				break;

			case 2:
				snprintf(buf, sizeof(buf), "WPA2 %s (%s)",
					 format_enc_suites(c->auth_suites),
					 format_enc_ciphers(c->pair_ciphers | c->group_ciphers));
				break;

			case 1:
				snprintf(buf, sizeof(buf), "WPA %s (%s)",
					 format_enc_suites(c->auth_suites),
					 format_enc_ciphers(c->pair_ciphers | c->group_ciphers));
				break;
			}
		} else {
			snprintf(buf, sizeof(buf), "none");
		}
	} else {
		snprintf(buf, sizeof(buf), "none");
	}

	return buf;
}

static char * format_hwmodes(int modes)
{
	static char buf[12];

	if (modes <= 0) {
		snprintf(buf, sizeof(buf), "unknown");
	} else {
		snprintf(buf, sizeof(buf), "802.11%s%s%s%s%s",
			 (modes & LWF_80211_A) ? "a" : "",
			 (modes & LWF_80211_B) ? "b" : "",
			 (modes & LWF_80211_G) ? "g" : "",
			 (modes & LWF_80211_N) ? "n" : "",
			 (modes & LWF_80211_AC) ? "ac" : "");
	}

	return buf;
}

static char * format_assocrate(struct lwf_rate_entry *r)
{
	static char buf[80];
	char *p = buf;
	int l = sizeof(buf);

	if (r->rate <= 0) {
		snprintf(buf, sizeof(buf), "unknown");
	} else {
		p += snprintf(p, l, "%s", format_rate(r->rate));
		l = sizeof(buf) - (p - buf);

		if (r->is_ht) {
			p += snprintf(p, l, ", MCS %d, %dMHz", r->mcs, r->mhz);
			l = sizeof(buf) - (p - buf);
		} else if (r->is_vht) {
			p += snprintf(p, l, ", VHT-MCS %d, %dMHz", r->mcs, r->mhz);
			l = sizeof(buf) - (p - buf);

			if (r->nss) {
				p += snprintf(p, l, ", VHT-NSS %d", r->nss);
				l = sizeof(buf) - (p - buf);
			}
		}
	}

	return buf;
}


static const char * print_type(const struct lwf_ops *iw, const char *ifname)
{
	const char *type = lwf_type(ifname);

	return type ? type : "unknown";
}

static char * print_hardware_id(const struct lwf_ops *iw, const char *ifname)
{
	static char buf[20];
	struct lwf_hardware_id ids;

	if (!iw->hardware_id(ifname, (char *)&ids)) {
		snprintf(buf, sizeof(buf), "%04X:%04X %04X:%04X",
			 ids.vendor_id, ids.device_id,
			 ids.subsystem_vendor_id, ids.subsystem_device_id);
	} else {
		snprintf(buf, sizeof(buf), "unknown");
	}

	return buf;
}

static char * print_hardware_name(const struct lwf_ops *iw, const char *ifname)
{
	static char buf[128];

	if (iw->hardware_name(ifname, buf))
		snprintf(buf, sizeof(buf), "unknown");

	return buf;
}

static char * print_txpower_offset(const struct lwf_ops *iw, const char *ifname)
{
	int off;
	static char buf[12];

	if (iw->txpower_offset(ifname, &off))
		snprintf(buf, sizeof(buf), "unknown");
	else if (off != 0)
		snprintf(buf, sizeof(buf), "%d dB", off);
	else
		snprintf(buf, sizeof(buf), "none");

	return buf;
}

static char * print_frequency_offset(const struct lwf_ops *iw, const char *ifname)
{
	int off;
	static char buf[12];

	if (iw->frequency_offset(ifname, &off))
		snprintf(buf, sizeof(buf), "unknown");
	else if (off != 0)
		snprintf(buf, sizeof(buf), "%.3f GHz", ((float)off / 1000.0));
	else
		snprintf(buf, sizeof(buf), "none");

	return buf;
}

static char * print_ssid(const struct lwf_ops *iw, const char *ifname)
{
	char buf[LWF_ESSID_MAX_SIZE + 1] = { 0 };

	if (iw->ssid(ifname, buf))
		memset(buf, 0, sizeof(buf));

	return format_ssid(buf);
}

static char * print_bssid(const struct lwf_ops *iw, const char *ifname)
{
	static char buf[18] = { 0 };

	if (iw->bssid(ifname, buf))
		snprintf(buf, sizeof(buf), "00:00:00:00:00:00");

	return buf;
}

static char * print_mode(const struct lwf_ops *iw, const char *ifname)
{
	int mode;
	static char buf[128];

	if (iw->mode(ifname, &mode))
		mode = LWF_OPMODE_UNKNOWN;

	snprintf(buf, sizeof(buf), "%s", LWF_OPMODE_NAMES[mode]);

	return buf;
}

static char * print_channel(const struct lwf_ops *iw, const char *ifname)
{
	int ch;

	if (iw->channel(ifname, &ch))
		ch = -1;

	return format_channel(ch);
}

static char * print_frequency(const struct lwf_ops *iw, const char *ifname)
{
	int freq;

	if (iw->frequency(ifname, &freq))
		freq = -1;

	return format_frequency(freq);
}

static char * print_txpower(const struct lwf_ops *iw, const char *ifname)
{
	int pwr, off;

	if (iw->txpower_offset(ifname, &off))
		off = 0;

	if (iw->txpower(ifname, &pwr))
		pwr = -1;
	else
		pwr += off;

	return format_txpower(pwr);
}

static char * print_quality(const struct lwf_ops *iw, const char *ifname)
{
	int qual;

	if (iw->quality(ifname, &qual))
		qual = -1;

	return format_quality(qual);
}

static char * print_quality_max(const struct lwf_ops *iw, const char *ifname)
{
	int qmax;

	if (iw->quality_max(ifname, &qmax))
		qmax = -1;

	return format_quality_max(qmax);
}

static char * print_signal(const struct lwf_ops *iw, const char *ifname)
{
	int sig;

	if (iw->signal(ifname, &sig))
		sig = 0;

	return format_signal(sig);
}

static char * print_noise(const struct lwf_ops *iw, const char *ifname)
{
	int noise;

	if (iw->noise(ifname, &noise))
		noise = 0;

	return format_noise(noise);
}

static char * print_rate(const struct lwf_ops *iw, const char *ifname)
{
	int rate;

	if (iw->bitrate(ifname, &rate))
		rate = -1;

	return format_rate(rate);
}

static char * print_encryption(const struct lwf_ops *iw, const char *ifname)
{
	struct lwf_crypto_entry c = { 0 };

	if (iw->encryption(ifname, (char *)&c))
		return format_encryption(NULL);

	return format_encryption(&c);
}

static char * print_hwmodes(const struct lwf_ops *iw, const char *ifname)
{
	int modes;

	if (iw->hwmodelist(ifname, &modes))
		modes = -1;

	return format_hwmodes(modes);
}

static char * print_mbssid_supp(const struct lwf_ops *iw, const char *ifname)
{
	int supp;
	static char buf[4];

	if (iw->mbssid_support(ifname, &supp))
		snprintf(buf, sizeof(buf), "no");
	else
		snprintf(buf, sizeof(buf), "%s", supp ? "yes" : "no");

	return buf;
}

static char * print_phyname(const struct lwf_ops *iw, const char *ifname)
{
	static char buf[32];

	if (!iw->phyname(ifname, buf))
		return buf;

	return "?";
}


static void print_info(const struct lwf_ops *iw, const char *ifname)
{
	printf("%-9s ESSID: %s\n",
	       ifname,
	       print_ssid(iw, ifname));
	printf("          Access Point: %s\n",
	       print_bssid(iw, ifname));
	printf("          Mode: %s  Channel: %s (%s)\n",
	       print_mode(iw, ifname),
	       print_channel(iw, ifname),
	       print_frequency(iw, ifname));
	printf("          Tx-Power: %s  Link Quality: %s/%s\n",
	       print_txpower(iw, ifname),
	       print_quality(iw, ifname),
	       print_quality_max(iw, ifname));
	printf("          Signal: %s  Noise: %s\n",
	       print_signal(iw, ifname),
	       print_noise(iw, ifname));
	printf("          Bit Rate: %s\n",
	       print_rate(iw, ifname));
	printf("          Encryption: %s\n",
	       print_encryption(iw, ifname));
	printf("          Type: %s  HW Mode(s): %s\n",
	       print_type(iw, ifname),
	       print_hwmodes(iw, ifname));
	printf("          Hardware: %s [%s]\n",
	       print_hardware_id(iw, ifname),
	       print_hardware_name(iw, ifname));
	printf("          TX power offset: %s\n",
	       print_txpower_offset(iw, ifname));
	printf("          Frequency offset: %s\n",
	       print_frequency_offset(iw, ifname));
	printf("          Supports VAPs: %s  PHY name: %s\n",
	       print_mbssid_supp(iw, ifname),
	       print_phyname(iw, ifname));
}


static void print_scanlist(const struct lwf_ops *iw, const char *ifname)
{
	int i, x, len;
	char buf[LWF_BUFSIZE];
	struct lwf_scanlist_entry *e;

	if (iw->scanlist(ifname, buf, &len)) {
		printf("Scanning not possible\n\n");
		return;
	} else if (len <= 0) {
		printf("No scan results\n\n");
		return;
	}

	for (i = 0, x = 1; i < len; i += sizeof(struct lwf_scanlist_entry), x++) {
		e = (struct lwf_scanlist_entry *)&buf[i];

		printf("Cell %02d - Address: %s\n",
		       x,
		       format_bssid(e->mac));
		printf("          ESSID: %s\n",
		       format_ssid(e->ssid));
		printf("          Mode: %s  Channel: %s\n",
		       LWF_OPMODE_NAMES[e->mode],
		       format_channel(e->channel));
		printf("          Signal: %s  Quality: %s/%s\n",
		       format_signal(e->signal - 0x100),
		       format_quality(e->quality),
		       format_quality_max(e->quality_max));
		printf("          Encryption: %s\n\n",
		       format_encryption(&e->crypto));
	}
}


static void print_txpwrlist(const struct lwf_ops *iw, const char *ifname)
{
	int len, pwr, off, i;
	char buf[LWF_BUFSIZE];
	struct lwf_txpwrlist_entry *e;

	if (iw->txpwrlist(ifname, buf, &len) || len <= 0) {
		printf("No TX power information available\n");
		return;
	}

	if (iw->txpower(ifname, &pwr))
		pwr = -1;

	if (iw->txpower_offset(ifname, &off))
		off = 0;

	for (i = 0; i < len; i += sizeof(struct lwf_txpwrlist_entry)) {
		e = (struct lwf_txpwrlist_entry *)&buf[i];

		printf("%s%3d dBm (%4d mW)\n",
		       (pwr == e->dbm) ? "*" : " ",
		       e->dbm + off,
		       lwf_dbm2mw(e->dbm + off));
	}
}


static void print_freqlist(const struct lwf_ops *iw, const char *ifname)
{
	int i, len, ch;
	char buf[LWF_BUFSIZE];
	struct lwf_freqlist_entry *e;

	if (iw->freqlist(ifname, buf, &len) || len <= 0) {
		printf("No frequency information available\n");
		return;
	}

	if (iw->channel(ifname, &ch))
		ch = -1;

	for (i = 0; i < len; i += sizeof(struct lwf_freqlist_entry)) {
		e = (struct lwf_freqlist_entry *)&buf[i];

		printf("%s %s (Channel %s)%s\n",
		       (ch == e->channel) ? "*" : " ",
		       format_frequency(e->mhz),
		       format_channel(e->channel),
		       e->restricted ? " [restricted]" : "");
	}
}


static void print_assoclist(const struct lwf_ops *iw, const char *ifname)
{
	int i, len;
	char buf[LWF_BUFSIZE];
	struct lwf_assoclist_entry *e;

	if (iw->assoclist(ifname, buf, &len)) {
		printf("No information available\n");
		return;
	} else if (len <= 0) {
		printf("No station connected\n");
		return;
	}

	for (i = 0; i < len; i += sizeof(struct lwf_assoclist_entry)) {
		e = (struct lwf_assoclist_entry *)&buf[i];

		printf("%s  %s / %s (SNR %d)  %d ms ago\n",
		       format_bssid(e->mac),
		       format_signal(e->signal),
		       format_noise(e->noise),
		       (e->signal - e->noise),
		       e->inactive);

		printf("	RX: %-38s  %8d Pkts.\n",
		       format_assocrate(&e->rx_rate),
		       e->rx_packets
		       );

		printf("	TX: %-38s  %8d Pkts.\n",
		       format_assocrate(&e->tx_rate),
		       e->tx_packets
		       );

		printf("	expected throughput: %s\n\n",
		       format_rate(e->thr));
	}
}


static char * lookup_country(char *buf, int len, int iso3166)
{
	int i;
	struct lwf_country_entry *c;

	for (i = 0; i < len; i += sizeof(struct lwf_country_entry)) {
		c = (struct lwf_country_entry *)&buf[i];

		if (c->iso3166 == iso3166)
			return c->ccode;
	}

	return NULL;
}

static void print_countrylist(const struct lwf_ops *iw, const char *ifname)
{
	int len;
	char buf[LWF_BUFSIZE];
	char *ccode;
	char curcode[3];
	const struct lwf_iso3166_label *l;

	if (iw->countrylist(ifname, buf, &len)) {
		printf("No country code information available\n");
		return;
	}

	if (iw->country(ifname, curcode))
		memset(curcode, 0, sizeof(curcode));

	for (l = LWF_ISO3166_NAMES; l->iso3166; l++) {
		if ((ccode = lookup_country(buf, len, l->iso3166)) != NULL) {
			printf("%s %4s	%c%c\n",
			       strncmp(ccode, curcode, 2) ? " " : "*",
			       ccode, (l->iso3166 / 256), (l->iso3166 % 256));
		}
	}
}

static void print_htmodelist(const struct lwf_ops *iw, const char *ifname)
{
	int i, htmodes = 0;

	if (iw->htmodelist(ifname, &htmodes)) {
		printf("No HT mode information available\n");
		return;
	}

	for (i = 0; i < ARRAY_SIZE(LWF_HTMODE_NAMES); i++)
		if (htmodes & (1 << i))
			printf("%s ", LWF_HTMODE_NAMES[i]);

	printf("\n");
}

static void lookup_phy(const struct lwf_ops *iw, const char *section)
{
	char buf[LWF_BUFSIZE];

	if (!iw->lookup_phy) {
		fprintf(stderr, "Not supported\n");
		return;
	}

	if (iw->lookup_phy(section, buf)) {
		fprintf(stderr, "Phy not found\n");
		return;
	}

	printf("%s\n", buf);
}


int main(int argc, char **argv)
{
	int i, rv = 0;
	char *p;
	const struct lwf_ops *iw;
	glob_t globbuf;

	if (argc > 1 && argc < 3) {
		fprintf(stderr,
			"Usage:\n"
			"	lwf <device> info\n"
			"	lwf <device> scan\n"
			"	lwf <device> txpowerlist\n"
			"	lwf <device> freqlist\n"
			"	lwf <device> assoclist\n"
			"	lwf <device> countrylist\n"
			"	lwf <device> htmodelist\n"
			"	lwf <backend> phyname <section>\n"
			);

		return 1;
	}

	if (argc == 1) {
		glob("/sys/class/net/*", 0, NULL, &globbuf);

		for (i = 0; i < globbuf.gl_pathc; i++) {
			p = strrchr(globbuf.gl_pathv[i], '/');

			if (!p)
				continue;

			iw = lwf_backend(++p);

			if (!iw)
				continue;

			print_info(iw, p);
			printf("\n");
		}

		globfree(&globbuf);
		return 0;
	}

	if (argc > 3) {
		iw = lwf_backend_by_name(argv[1]);

		if (!iw) {
			fprintf(stderr, "No such wireless backend: %s\n", argv[1]);
			rv = 1;
		} else {
			switch (argv[2][0]) {
			case 'p':
				lookup_phy(iw, argv[3]);
				break;

			default:
				fprintf(stderr, "Unknown command: %s\n", argv[2]);
				rv = 1;
			}
		}
	} else {
		iw = lwf_backend(argv[1]);

		if (!iw) {
			fprintf(stderr, "No such wireless device: %s\n", argv[1]);
			rv = 1;
		} else {
			for (i = 2; i < argc; i++) {
				switch (argv[i][0]) {
				case 'i':
					print_info(iw, argv[1]);
					break;

				case 's':
					print_scanlist(iw, argv[1]);
					break;

				case 't':
					print_txpwrlist(iw, argv[1]);
					break;

				case 'f':
					print_freqlist(iw, argv[1]);
					break;

				case 'a':
					print_assoclist(iw, argv[1]);
					break;

				case 'c':
					print_countrylist(iw, argv[1]);
					break;

				case 'h':
					print_htmodelist(iw, argv[1]);
					break;

				default:
					fprintf(stderr, "Unknown command: %s\n", argv[i]);
					rv = 1;
				}
			}
		}
	}

	lwf_finish();

	return rv;
}
