(local os (require :os))

(local lwf (. (require :lwf)))

(fn printf [fmt ...]
  (print (string.format fmt ...)))

(fn s [x]
  (if (= x nil) "?"
      (tostring x)))

(fn n [x]
  (if (= x nil) 0
      (tonumber x)))

(fn hwmode [lwf dev]
  (let [m (lwf.hwmodelist dev)]
    (printf "hwmode: m = %s" (s m))
    (if m
        (let [s :802.11]
          (when m.a (local s (.. s :a)))
          (when m.b (local s (.. s :b)))
          (when m.g (local s (.. s :g)))
          (when m.n (local s (.. s :n)))
          s)
        "?")))

(fn print-info [api dev]
  (let [lwf (. lwf api)
        enc (lwf.encryption dev)]
    (printf "print-info: api = %s, dev = %s" (s api) (s dev))
    (printf "print-info: lwf = %s, enc = %s" (s lwf) (s enc))
    (printf "%-9s ESSID: \"%s\"" dev (s (lwf.ssid dev)))
    (printf "          Access Point: %s" (s (lwf.bssid dev)))
    (printf "          Type: %s  HW Mode(s): %s" api (hwmode lwf dev))
    (printf "          Mode: %s  Channel: %d (%.3f GHz)" (s (lwf.mode dev))
            (n (lwf.channel dev)) (/ (n (lwf.frequency dev)) 1000))
    (printf "          Tx-Power: %s dBm  Link Quality: %s/%s"
            (s (lwf.txpower dev)) (s (lwf.quality dev))
            (s (lwf.quality_max dev)))
    (printf "          Signal: %s dBm  Noise: %s dBm" (s (lwf.signal dev))
            (s (lwf.noise dev)))
    (printf "          Bit Rate: %.1f MBit/s" (/ (n (lwf.bitrate dev)) 1000))
    (printf "          Encryption: %s" (s (or (and enc enc.description) :None)))
    (printf "          Supports VAPs: %s"
            (or (and (lwf.mbssid_support dev) :yes) :no))
    (print "")))

(fn print-scan [api dev]
  (let [lwf (. lwf api)
        sr (lwf.scanlist dev)]
    (printf "print-scan: api = %s, dev = %s" (s api) (s dev))
    (printf "print-scan: lwf = %s, sr = %s" (s lwf) (s sr))
    (if (and sr (> (length sr) 0))
        (each [si se (ipairs sr)]
          (printf "Cell %02d - Address: %s" si se.bssid)
          (printf "          ESSID: \"%s\"" (s se.ssid))
          (printf "          Mode: %s  Channel: %d" (s se.mode) (n se.channel))
          (printf "          Signal: %s dBm  Quality: %d/%d" (s se.signal)
                  (n se.quality) (n se.quality_max))
          (printf "          Encryption: %s"
                  (s (or se.encryption.description :None)))
          (print ""))
        (do
          (print "No scan results or scanning not possible")
          (print "")))))

(fn print-txpwrlist [api dev]
  (let [lwf (. lwf api)
        pl (lwf.txpwrlist dev)
        cp (n (lwf.txpower dev))]
    (printf "print-txpwrlist: api = %s, dev = %s" (s api) (s dev))
    (printf "print-txpwrlist: lwf = %s, pl = %s, cp = %s" (s lwf) (s pl) (s cp))
    (if (and pl (> (length pl) 0))
        (each [_ pe (ipairs pl)]
          (printf "%s%3d dBm (%4d mW)" (or (and (= cp pe.dbm) "*") " ")
                  (n pe.dbm) (n pe.mw)))
        (print "No TX power information available"))
    (print "")))

(fn print-freqlist [api dev]
  (let [lwf (. lwf api)
        fl (lwf.freqlist dev)
        cc (n (lwf.channel dev))]
    (printf "print-freqlist: api = %s, dev = %s" (s api) (s dev))
    (printf "print-freqlist: lwf = %s, fl = %s, cc = %s" (s lwf) (s fl) (s cc))
    (if (and fl (> (length fl) 0))
        (each [_ fe (ipairs fl)]
          (printf "%s %.3f GHz (Channel %d)%s"
                  (or (and (= cc fe.channel) "*") " ") (/ (n fe.mhz) 1000)
                  (n fe.channel) (or (and fe.restricted " [restricted]") "")))
        (print "No frequency information available"))
    (print "")))

(fn print-assoclist [api dev]
  (let [lwf (. lwf api)
        al (lwf.assoclist dev)]
    (printf "print-assoclist: api = %s, dev = %s" (s api) (s dev))
    (printf "print-assoclist: lwf = %s, al = %s" (s lwf) (s al))
    (if (and al (next al))
        (each [ai ae (pairs al)]
          (printf "%s  %s dBm" ai (s ae.signal)))
        (print "No client connected or no information available"))
    (print "")))

;; repl output is these two sexprs getting eval'd
;; (let [sl (lwf.scanlist :wlp2s0)]
;;   (. sl 1)) ;; grab first from scanlist array
;;
;; (let [hwml (lwf.hwmodelist :wlp2s0)]
;;   (printf "%s" (s hwml)))

(print-info :nl80211 :wlp2s0)
(print-scan :nl80211 :wlp2s0)
(print-txpwrlist :nl80211 :wlp2s0)
(print-freqlist :nl80211 :wlp2s0)
(print-assoclist :nl80211 :wlp2s0)
(print-info :nl80211 :wlp2s0)
