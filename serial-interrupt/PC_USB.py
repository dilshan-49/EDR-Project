import serial
import struct
import time

# Configure the serial port
SERIAL_PORT = "COM10"  # Replace with the correct COM port for your device
BAUD_RATE = 9600      # Match the baud rate configured in your microcontroller

def send_coordinate(ser, x_coordinate,y_coordinate):
    """
    Sends the target pulse count to the microcontroller.
    """
    # Convert the 16-bit coordinate to two bytes
    data = struct.pack(">HH", x_coordinate,y_coordinate)  # Big-endian 16-bit integer
    ser.write(data)
    print(f"Sent target cordinates: { x_coordinate},{y_coordinate}")

def read_response(ser):
    """
    Reads the response from the microcontroller.
    """
    response = ser.readline().decode("utf-8").strip()
    print(f"Response from microcontroller: {response}")

def main():
    # Open the serial port
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=5)
        while True:
            time.sleep(0.1) # Wait for the connection to initialize

            response = ser.read(1)  # Read 1 byte
            if response == b'\xFF':
                print("SUCCESS")
            elif response == b'\x00':
                print("ERROR")
            else:
                print(f"Unknown response: {response}")

    except serial.SerialException as e:
        print(f"Error: {e}")
    finally:
        if ser.is_open:
            ser.close()

if __name__ == "__main__":
    main()