from requests import get
from flask import Flask, request
import base64
from io import BytesIO
from matplotlib.figure import Figure
import csv_test

# Global Variables


# print public IP address
ip = get('https://api.ipify.org').text
print(f'\nPUBLIC IP: {ip}\n')

# Flask Server
app = Flask(__name__)


@app.route("/")
def hello():
    # data variables
    temp = request.args.get("temp")
    humidity = request.args.get("humidity")
    state = request.args.get("state")

    # terminal output
    print(f'temp    : {temp}')
    print(f'humidity: {humidity}')
    print(f'state   : {state}')
    print()

    # text response
    ret = "We received temp: " + \
        str(request.args.get("temp")) + ', humidity: ' + \
        str(request.args.get("humidity"))

    # save to csv
    csv_test.append_csv(temp, humidity, state)
    df = csv_test.load_csv()

    return ret


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



@app.route("/stats/")
def hello2():
    # load csv
    df = csv_test.load_csv()

    # Generate the figure **without using pyplot**.
    fig = Figure()
    ax = fig.subplots()
    ax.plot(df['timestamp'], df['temp'])
    # Save it to a temporary buffer.
    buf = BytesIO()
    fig.savefig(buf, format="png")
    # Embed the result in the html output.
    data = base64.b64encode(buf.getbuffer()).decode("ascii")
    return f"<img src='data:image/png;base64,{data}'/>"
