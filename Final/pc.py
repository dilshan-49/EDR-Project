# You need to install pyserial: pip install pyserial
import serial
import time

# Update this to match your COM port (e.g., 'COM3' on Windows, '/dev/ttyACM0' on Linux)
PORT = 'COM11'
BAUD = 9600  # Baud rate is ignored for USB CDC, but required by pyserial

def main():
    with serial.Serial(PORT, BAUD, timeout=1) as ser:
        time.sleep(2)  # Wait for device to reset after opening port

        for value in range(10, 20):
            ser.write(bytes([value]))
            time.sleep(1)
            response = ser.read(5)  # m_usb_tx_uint sends 5 ASCII digits
            print(f"Sent: {value}, Received: {response.decode(errors='ignore')}")

if __name__ == "__main__":
    main()