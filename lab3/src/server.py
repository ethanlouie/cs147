from requests import get
from flask import Flask
from flask import request

# print public IP address
ip = get('https://api.ipify.org').text
print(f'\nPUBLIC IP: {ip}\n')

# Flask Server
app = Flask(__name__)
@app.route("/")
def hello():
	print(request.args.get("temp"))
	print(request.args.get("humidity"))
	return "We received temp: "+str(request.args.get("temp")) + ', humidity: ' +str(request.args.get("humidity"))

