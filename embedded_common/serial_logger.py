import serial
from datetime import datetime

port = "COM4"
baud = 115200
ser = serial.Serial(port, baud)
ser.flushInput()

start_time = datetime.now().strftime('%Y_%m_%d-%I_%M_%S_%p')

while True:
    try:
        ser_bytes = ser.readline()
        # decoded_bytes = float(ser_bytes[0:len(ser_bytes)-2].decode("utf-8"))
        print(ser_bytes.decode("utf-8"))
        with open(f"dns_logs_{start_time}.log","a") as f:
            f.write(f"{datetime.now()}: {ser_bytes.decode('utf-8')}")

    except Exception as e:
        print(e)
        print("Keyboard Interrupt")
        break