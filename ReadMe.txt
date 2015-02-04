Upload.py is the python script that calls the curl command.

The curl command takes four arguments: The database's web address, the username/pass for basic authentication, the data, and the location of the security certificate for the database. Data must be provided as a JSON string.

ADylosDuinoV2_5 is the arduino code that calls upload.py and gives is the arguments for the curl command.  The arguments are the name of the instrument, a timestamp, and values for the particle count in each of the four size bins and the temp/humidity.  It is written in C.