from requests import get
from flask import Flask
from flask import request

# print public IP address
print(f'Public IP: {get('https://api.ipify.org').text}')

# Flask Server
app = Flask(__name__)
@app.route("/")
def hello():
	print(request.args.get("temp"))
	print(request.args.get("humidity"))
	return "We received temp: "+str(request.args.get("temp")) + ', humidity: ' +str(request.args.get("humidity"))

