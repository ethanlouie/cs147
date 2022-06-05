from requests import get
from flask import Flask, request, render_template
import json
from apscheduler.schedulers.background import BackgroundScheduler
import sqlite3 as lite

from matplotlib.figure import Figure
from matplotlib.pyplot import MaxNLocator
from io import BytesIO
import base64
import pandas as pd
from time import time
import datetime

starting_timestamp = int(time())
schedule = {}


# print public IP address
ip = get('https://api.ipify.org').text
print(f'\nPUBLIC IP: {ip}\n')

#assume 0.5 degree loss in either direction always
#assume 1 degree heating/cooling per half hour
#if cold now and get hot during day but want cold, optimize towards min
#if cold now and get colder during day but want cold, do nothing, but heat sometimes
#if cold now and get hot during day but want hot, do nothing but cool sometimes
#if cold now and get colder during day but want hot, optimize towards max

def calculate_optimal_schedule(weather_data: json = None):
	global schedule
	global min
	global max
	schedule = {}
	if weather_data == None:
		with open ('weather.json', 'r') as file:
			weather_data = json.load(file)
		
	offset = weather_data["hourly"][0]["dt"]
	resp = cur.execute("SELECT * FROM sensor_data ORDER BY timestamp DESC LIMIT 1;")
	row = resp.fetchone()
	print(row)
	print(row[0])
	print(row[1])
	res = [(hour['dt'], hour['temp']) for hour in weather_data["hourly"]]
	warm = (res[len(res)//2][1] > res[0][1] or res[len(res)//2+1][1] > res[0][1])

	stop = (max-min) * 3600
	#stop = min(3600 * 4, (max-min) * 3600)
	stop //= 12 
	begin_countdown = False

	current_temp = row[1]


	#simplify for prototype
	if warm: #optimize towards min
		for time, temp in res:
			for i in range(12):
				_5_min = time + i * 300
				if begin_countdown == True:
					if stop == 0:
						begin_countdown = False
						stop = (max-min) * 3600
						stop //= 12 
					else:
						stop -= 1
					schedule[_5_min] = 0
					continue
				if current_temp > min:
					schedule[_5_min] = 1
					current_temp -= 1 * 2 / 12
				else:
					schedule[_5_min] = 0
					begin_countdown = True

	

	else: #optimize towards max since cold
		for time, temp in res:
			for i in range(12):
				_5_min = time + i * 300
				if begin_countdown == True:
					if stop == 0:
						begin_countdown = False
						stop = (max-min) * 3600
						stop //= 12 
					else:
						stop -= 1
					schedule[_5_min] = 0
					continue
				if current_temp < max:
					schedule[_5_min] = 1
					current_temp += 1 * 2 / 12
				else:
					schedule[_5_min] = 0
					begin_countdown = True


	print(schedule)


		




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
min = 0
max = 100
with open("min_file.txt", "r") as m:
	min = int(m.readline())

with open("max_file.txt", "r") as m:
	max = int(m.readline())



@app.route("/")
def hello():
	return render_template("index.html", min=min, max=max)
	# print(request.args.get("temp"))
	# print(request.args.get("humidity"))
	# return "We received temp: "+str(request.args.get("temp")) + ', humidity: ' +str(request.args.get("humidity"))

@app.route("/update_pref")
def update_preferences():
	global min
	global max
	temp_min = request.args.get("min")
	temp_max = request.args.get("max")

	if temp_max > temp_min:

		with open("min_file.txt", "w+") as m:
			min = temp_min
			m.write(min)

		with open("max_file.txt", "w+") as m:
			max = temp_max
			m.write(max)

	return render_template("index.html", min=min, max=max)

@app.route("/schedule")
def get_schedule():
	return schedule
	


@app.route("/data", methods=["GET"])
def process_data():
	temp = request.args.get("temp")
	humidity = request.args.get("humidity")
	state = request.args.get("state")
	cur = conn.cursor()

	# cur.execute("INSERT INTO sensor_data VALUES (datetime('now'), {0}, {1}".format(temp, humidity))
	cur.execute("INSERT INTO sensor_data VALUES (datetime('now'), (?), (?), (?))", (temp, humidity, state))
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


@app.route("/temp/")
def hello2():
    # load csv
	df = pd.read_sql_query("SELECT * FROM sensor_data WHERE " \
					"timestamp > datetime('now', '-7 day');", conn)

    # Generate the figure **without using pyplot**.
	fig = Figure()
	ax = fig.subplots() # use arguments to make 2x2 grid

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

@app.route("/humidity/")
def hello3():
    # load csv
	df = pd.read_sql_query("SELECT * FROM sensor_data WHERE " \
					"timestamp > datetime('now', '-7 day');", conn)

    # Generate the figure **without using pyplot**.
	fig = Figure()
	ax = fig.subplots() # use arguments to make 2x2 grid

	print(df['timestamp'][0])
	ax.plot(df['timestamp'], df['humidity'])
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

@app.route("/energy/")
def hello4():
    # load csv
	df = pd.read_sql_query("SELECT * FROM sensor_data WHERE " \
					"timestamp > datetime('now', '-7 day');", conn)

    # Generate the figure **without using pyplot**.
	fig = Figure()
	ax = fig.subplots() # use arguments to make 2x2 grid

	cost = (0,4000,1500)
	x = list()

	print(df['timestamp'][0])
	for i in df['state']:
		if i == None:
			i = 0

		x.append(int(i))

	ax.plot(df['timestamp'], x)
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
