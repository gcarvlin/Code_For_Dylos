import subprocess
import sys
import json

# Initialize variables
url="https://replicant.deohs.washington.edu/api/v1/datapoints"
cert="/etc/ssl/certs/cacert4.pem"
auth="USERNAME:PASS"

# Functions
def prepdata(value, tags):
    "This prepares a data structure to send in JSON format"
    data = [
               {
                   "name" :      sys.argv[1], 
                   "timestamp" : sys.argv[2] + "000", 
                   "value":      value, 
                   "tags" :      tags
               }
           ]
    return data

# Create data structures
bin1 = prepdata(sys.argv[3], { "Bin" : "bin1" })
bin2 = prepdata(sys.argv[4], { "Bin" : "bin2" })
bin3 = prepdata(sys.argv[5], { "Bin" : "bin3" })
bin4 = prepdata(sys.argv[6], { "Bin" : "bin4" })
temp = prepdata(sys.argv[7] + "." + sys.argv[8], { "Temp" : "temp" })
rh = prepdata(sys.argv[9] + "." + sys.argv[10], { "RH" : "rh" })

# Send data in JSON format using curl
items = [bin1, bin2, bin3, bin4, temp, rh]
for item in items:
    subprocess.call(["curl",url,"-u",auth,"-d",json.dumps(item),"--cacert",cert])
