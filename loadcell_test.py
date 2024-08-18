import serial
import time

PORT="COM6"
BAUD=115200

def get_weight(s: serial.Serial):
    w = [0x01, 0x03, 0x00, 0x01, 0x00, 0x02, 0x95, 0xCB]
    ser.write(w)
    time.sleep(1)
    rx = ser.read_all()
    print("Weight")
    print(f"TX'd: {w}")
    print(f"RX'd: {rx}")


def set_zero(s: serial.Serial):
    ...


def set_gravity(s: serial.Serial):
    ...


def set_id_rate(s: serial.Serial):
    ...


def set_filter_param(s: serial.Serial):
    ...


def set_address(s: serial.Serial):
    ...


def set_tare_zero(s: serial.Serial):
    ...


if __name__ == "__main__":
    ser = serial.Serial(PORT, BAUD, timeout=1)
    get_weight(ser)