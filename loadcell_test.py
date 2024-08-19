import serial
import time

PORT="COM6"
BAUD=115200

def get_weight(s: serial.Serial):
    w = [0x01, 0x03, 0x00, 0x01, 0x00, 0x02, 0x95, 0xCB]
    ser.write(w)
    time.sleep(1)
    rx = ser.read_all()
    ser.flush()
    print("Weight")
    print(f"TX'd: {w}")
    print(f"RX'd: {rx}")


def set_zero(s: serial.Serial):
    w = [0x01, 0x10, 0x00, 0x01, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x01, 0xF3, 0xA3]
    ser.write(w)
    time.sleep(1)
    rx = ser.read_all()
    ser.flush()
    print("Zero")
    print(f"TX'd: {w}")
    print(f"RX'd: {rx}")


def set_gravity(s: serial.Serial):
    w = [0x01, 0x10, 0x00, 0x09, 0x00, 0x02, 0x04, 0x00, 0x01, 0x7E, 0x9A, 0xC3, 0xCE]
    ser.write(w)
    time.sleep(1)
    rx = ser.read_all()
    ser.flush()
    print("Gravity")
    print(f"TX'd: {w}")
    print(f"RX'd: {rx}")



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
    time.sleep(1)
    set_zero(ser)
    time.sleep(1)
    set_gravity(ser)
    time.sleep(1)