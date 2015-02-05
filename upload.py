import subprocess
import sys
import json

url="https://replicant.deohs.washington.edu/api/v1/datapoints"
cert="/etc/ssl/certs/cacert4.pem"
auth="USERNAME:PASS"

def prepdata(value, tags):
    "This prepares a data structure to send in JSON format"
    data = {
               "name" :      sys.argv[1], 
               "timestamp" : sys.argv[2] + "000", 
               "value":      value, 
               "tags" :      tags
           }
    return [data]

def prepbin(n):
    "This prepares data for a bin."
    return prepdata(sys.argv[n+2], { "Bin" : "bin" + str(n) })

# Prepare the bin data
bins = []
for n in range(0, 4):
    bins.append(prepbin(n+1))

# Prepare temperature and relative humidity data
temp = prepdata(sys.argv[7] + "." + sys.argv[8], { "Temp" : "temp" })
rh = prepdata(sys.argv[9] + "." + sys.argv[10], { "RH" : "rh" })

items = bins + [temp, rh]
for item in items:
    subprocess.call(["curl",url,"-u",auth,"-d",json.dumps(item),"--cacert",cert])
