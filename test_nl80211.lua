-- Load the nl80211 module
local iwi = require("lwf")

-- Assuming nl80211 has some functions, replace 'some_function' with an actual function from nl80211
-- Here, we will just print the table to verify it loads correctly
print(iwi.nl80211)

-- For demonstration, list all keys (functions/attributes) in the nl80211 module
for key, value in pairs(iwi.nl80211) do
    print(key, value)
end
