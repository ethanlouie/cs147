
from flask import Flask
from flask import request

app = Flask(__name__)

@app.route("/")
def hello():
	print(request.args.get("temp"))
	print(request.args.get("humidity"))
	return "We received temp: "+str(request.args.get("temp")) + ', humidity: ' +str(request.args.get("humidity"))

