from requests import get
from flask import Flask, request
import json
from apscheduler.schedulers.background import BackgroundScheduler
import sqlite3 as lite

from matplotlib.figure import Figure
from matplotlib.pyplot import MaxNLocator
from io import BytesIO
import base64
import pandas as pd
from time import time


# print public IP address
ip = get('https://api.ipify.org').text
print(f'\nPUBLIC IP: {ip}\n')


#for now, storing in json as proof of concept
def fetch_daily_weather():
	api_credential_file = open("../api_credentials.txt", "r")
	api_key = api_credential_file.readline()
	api_credential_file.close()

	url = ("https://api.openweathermap.org/data"
			"/2.5/onecall?lat=33.68&lon=-117.83"
			"&exclude=current,minutely,daily,alerts&appid="
		)
	url_with_key = url + api_key

	print("getting url with key")
	out_file = open("weather.json", "w")
	response = get(url_with_key)
	body = response.json()
	json.dump(body, out_file, indent = 6)
	out_file.close()


scheduler = BackgroundScheduler(timezone="America/Los_Angeles")
job = scheduler.add_job(fetch_daily_weather, 'cron', hour=0, minute=1)
scheduler.start()
conn = lite.connect("../storage.db", check_same_thread=False)

cur = conn.cursor()

cur.execute("CREATE TABLE IF NOT EXISTS sensor_data (timestamp DATETIME, temp NUMERIC, humidity NUMERIC, state NUMERIC);")
conn.commit()

# Flask Server
app = Flask(__name__)


@app.route("/")
def hello():
	print(request.args.get("temp"))
	print(request.args.get("humidity"))
	return "We received temp: "+str(request.args.get("temp")) + ', humidity: ' +str(request.args.get("humidity"))


@app.route("/data", methods=["POST"])
def process_data():
	temp = request.form.get("temp")
	humidity = request.form.get("humidity")
	state = request.form.get("state")
	cur = conn.cursor()

	# cur.execute("INSERT INTO sensor_data VALUES (datetime('now'), {0}, {1}".format(temp, humidity))
	cur.execute("INSERT INTO sensor_data VALUES (datetime('now', '-7 day'), (?), (?), (?))", (temp, humidity, state))
	conn.commit()

	return {
		"response": 200,
	}

	#return "200 OK temp={0}, humidity={1}, state={2}".format(temp, humidity, state)


@app.route("/graph", methods=["GET"])
def graph_data():
	# cur = conn.cursor()

	resp = cur.execute("SELECT * FROM sensor_data WHERE timestamp > datetime('now', '-7 day');")

	# get individual row data (sqlite3 row object)
	# for row in resp:
	# 	print(row)
		#return str(row)

	return str(resp.fetchall())


@app.route("/stats/")
def hello2():
    # load csv
	df = pd.read_sql_query("SELECT * FROM sensor_data WHERE " \
					"timestamp > datetime('now', '-7 day');", conn)

    # Generate the figure **without using pyplot**.
	fig = Figure()
	ax = fig.subplots()
	print(df['timestamp'][0])
	ax.plot(df['timestamp'], df['temp'])
	ax.set_xlabel('Time')
	ax.set_ylabel('Tempurature (Celcius)')
	#create maximum number of points in graph axes
	ax.xaxis.set_major_locator(MaxNLocator(5))
    # Save it to a temporary buffer.
	buf = BytesIO()
	fig.savefig(buf, format="png")
    # Embed the result in the html output.
	data = base64.b64encode(buf.getbuffer()).decode("ascii")
	return f"<img src='data:image/png;base64,{data}'/>"


# from matplotlib.figure import Figure
# from io import BytesIO
# import base64
# from flask import Flask, request
# from requests import get
# import pandas as pd
# from time import time

# filename = '/home/ubuntu/cs147/lab3/src/data_test.csv'


# def load_csv():
#     return pd.read_csv(filename)


# def save_csv(df):
#     df.to_csv(filename, index=False)
#     return


# def append_csv(temp, humidity, state):
#     with open(filename, 'a') as f:
#         f.write(f'\n{time()},{temp},{humidity},{state}')
#     return


# def append_csv2(temp, humidity, state):
#     df = load_csv()
#     df.append({'timestamp': [time()],
#                'temp': [temp],
#                'humidity': [humidity],
#                'state': [state]},
#               ignore_index=True)
#     return


# # Global Variables

# # print public IP address
# ip = get('https://api.ipify.org').text
# print(f'\nPUBLIC IP: {ip}\n')

# # Flask Server
# app = Flask(__name__)


# @app.route("/")
# def hello():
#     # data variables
#     temp = request.args.get("temp")
#     humidity = request.args.get("humidity")
#     state = request.args.get("state")

#     # terminal output
#     print(f'temp    : {temp}')
#     print(f'humidity: {humidity}')
#     print(f'state   : {state}')
#     print()

#     # text response
#     ret = "We received temp: " + \
#         str(request.args.get("temp")) + ', humidity: ' + \
#         str(request.args.get("humidity"))

#     # save to csv
#     append_csv(temp, humidity, state)
#     df = load_csv()

#     return ret

    # # Generate the figure **without using pyplot**.
    # fig = Figure()
    # ax = fig.subplots()
    # ax.plot(df['timestamp'], df['temp'])
    # # Save it to a temporary buffer.
    # buf = BytesIO()
    # fig.savefig(buf, format="png")
    # # Embed the result in the html output.
    # data = base64.b64encode(buf.getbuffer()).decode("ascii")
    # return ret + f"<img src='data:image/png;base64,{data}'/>"


# @app.route("/stats/")
# def hello2():
#     # load csv
#     df = load_csv()

#     # Generate the figure **without using pyplot**.
#     fig = Figure()
#     ax = fig.subplots()
#     ax.plot(df['timestamp'], df['temp'])
#     ax.set_xlabel('Time')
#     ax.set_ylabel('Tempurature (Celcius)')
#     # Save it to a temporary buffer.
#     buf = BytesIO()
#     fig.savefig(buf, format="png")
#     # Embed the result in the html output.
#     data = base64.b64encode(buf.getbuffer()).decode("ascii")
#     return f"<img src='data:image/png;base64,{data}'/>"
