import serial
import struct
import time

# Configure the serial port
SERIAL_PORT = "COM10"  # Replace with the correct COM port for your device
BAUD_RATE = 9600      # Match the baud rate configured in your microcontroller

def send_pulse_count(ser, pulse_count):
    """
    Sends the target pulse count to the microcontroller.
    """
    # Convert the 16-bit pulse count to two bytes
    data = struct.pack(">H", pulse_count)  # Big-endian 16-bit integer
    ser.write(data)
    print(f"Sent target pulse count: {pulse_count}")

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

        # Send the target pulse count
        target_pulse_count = 20  # Replace with the desired pulse count
        send_pulse_count(ser, target_pulse_count)

        # Wait for and read the response
        read_response(ser)

    except serial.SerialException as e:
        print(f"Error: {e}")
    finally:
        if ser.is_open:
            ser.close()

if __name__ == "__main__":
    main()