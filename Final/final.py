import serial
import serial.tools.list_ports
import time
import argparse
import sys

# Protocol constants matching firmware
SUCCESS = 0xFF
ERROR = 0x00
ACK = 0xAA
NAK = 0x55
GENERATING = 0x11
WAITING = 0x22

def find_mcu_port():
    """Find the serial port connected to the microcontroller."""
    ports = list(serial.tools.list_ports.comports())
    # Look for likely USB serial devices
    for port in ports:
        # Common descriptors for ATmega32U4 devices
        if any(id_str in port.description.lower() for id_str in ["arduino", "atmega", "usb", "serial"]):
            return port.device
    return None

def send_coordinates(ser, x, y):
    """Send two 16-bit coordinates to MCU as 4 bytes."""
    # Convert to 16-bit values and send high byte then low byte
    data = bytes([
        (x >> 8) & 0xFF,  # High byte of x
        x & 0xFF,         # Low byte of x
        (y >> 8) & 0xFF,  # High byte of y
        y & 0xFF          # Low byte of y
    ])
    ser.write(data)
    print(f"Sent: {' '.join([f'{b:02X}' for b in data])}")

def read_status(ser):
    """Read a single byte status code from the MCU"""
    if ser.in_waiting >= 1:
        raw_data = ser.read(1)
        # No conversion needed, return the byte value directly
        return raw_data[0]
    return None

def main():
    parser = argparse.ArgumentParser(description='Test the MCU coordinate pulse generator')
    parser.add_argument('--port', help='Serial port to use (auto-detect if not specified)')
    parser.add_argument('--baud', type=int, default=9600, help='Baud rate (default: 9600)')
    args = parser.parse_args()
    
 
    print(f"Connecting to MCU on {args.port} at {args.baud} baud...")
    try:
        ser = serial.Serial(args.port, args.baud, timeout=0.5)
        print("Connected successfully!")
    except Exception as e:
        print(f"Error opening serial port: {e}")
        return
    
    print("\nWaiting for MCU to initialize...")
    time.sleep(1)  # Give MCU time to initialize
    
    try:
        while True:
            # Clear any existing data in the buffer
            ser.reset_input_buffer()
            
            print("\n=== WAITING FOR MCU READY STATE ===")
            # Wait for the WAITING status from MCU
            waiting_found = False
            timeout_counter = 0
            while not waiting_found and timeout_counter < 100:
                status = read_status(ser)
                if status == WAITING:
                    print("MCU is ready and waiting for coordinates.")
                    waiting_found = True
                elif status is not None:
                    print(f"Received status: 0x{status:02X}")
                time.sleep(0.1)
                timeout_counter += 1
            
            if not waiting_found:
                print("Warning: Did not receive WAITING status. MCU may not be ready.")
                response = input("Continue anyway? (y/n): ")
                if response.lower() != 'y':
                    continue
            

            # Get coordinates from user
            print("\n=== ENTER COORDINATES ===")
            try:
                x = int(input("Enter X coordinate (0-65535): "))
                y = int(input("Enter Y coordinate (0-65535): "))
                if not (0 <= x <= 65535 and 0 <= y <= 65535):
                    print("Error: Coordinates must be in range 0-65535")
                    continue
            except ValueError:
                print("Error: Invalid input. Please enter numeric values.")
                continue
            
            # Send coordinates to MCU
            print("\n=== SENDING COORDINATES ===")
            
            send_coordinates(ser, x, y)
            time.sleep(0.09)
            ser.reset_input_buffer()
            
            # Wait for and display the echoed coordinates
            print("Waiting for echo response...")
            echo = b''
            timeout = time.time() + 2.0  # 2 second timeout
            while len(echo)<4 and time.time() < timeout:
                if ser.in_waiting > 0:
                    echo += ser.read(min(ser.in_waiting, 4 - len(echo)))
                time.sleep(0.1)
            
            if len(echo)==4:
                print(f"MCU echoed: 0x{echo.hex().upper()}")
            else:
                print("Warning: No echo received from MCU")
            
            # Get user decision to send ACK or NAK
            print("\n=== SEND RESPONSE ===")
            response = input("Send ACK (a) or NAK (n)? ").lower()
            
            if response == 'a':
                print("Sending ACK...")
                ser.write(bytes([ACK]))
                
                # Monitor for GENERATING and SUCCESS messages
                print("\n=== MONITORING MCU STATUS ===")
                status_timeout = time.time() + 30.0  # 30 second timeout
                
                while time.time() < status_timeout:

                    status = read_status(ser)
                    # Process only the latest status
                    if status == GENERATING:
                        print("MCU status: GENERATING pulses...")
                    elif status == SUCCESS:
                        print("MCU status: SUCCESS - Operation completed!")
                        break
                    elif status is not None:
                        print(f"MCU status: Unknown (0x{status:02X})")
                    else:
                        print("MCU status: No Response")
                    time.sleep(0.2)
                
                if time.time() >= status_timeout:
                    print("Warning: Timeout waiting for SUCCESS status")
            
            elif response == 'n':
                print("Sending NAK...")
                ser.write(bytes([NAK]))
                print("MCU should blink LED to indicate error")
            else:
                print("Invalid input, no response sent.")
            
            print("\n=== READY FOR NEXT COMMAND ===")
            input("Press Enter to continue...")
            
    except KeyboardInterrupt:
        print("\nExiting...")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        ser.close()
        print("Serial connection closed.")

if __name__ == "__main__":
    main()