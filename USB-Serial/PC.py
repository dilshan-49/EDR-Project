import serial
import struct
import time

# Configure the serial port
SERIAL_PORT = "COM10"  # Replace with the correct COM port for your device
BAUD_RATE = 9600      # Match the baud rate configured in your microcontroller

def send_message(ser, message):
    ser.write(message.encode('utf-8'))
    print(f"Message sent to microcontroller: {message}")

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
        time.sleep(2)  # Wait for the connection to initialize

        message="Hello, microcontroller!"
        send_message(ser, message)

        # Wait for and read the response
        read_response(ser)

    except serial.SerialException as e:
        print(f"Error: {e}")
    finally:
        if ser.is_open:
            ser.close()

if __name__ == "__main__":
    main()