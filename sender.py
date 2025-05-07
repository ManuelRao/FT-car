#  --- UNCHECKED CODE ---
import pygame
import serial
import time

# Initialize Pygame and the joystick
pygame.init()
pygame.joystick.init()

# Check if a joystick is connected
if pygame.joystick.get_count() == 0:
    print("No joystick connected.")
    exit()

# Initialize the first joystick
joystick = pygame.joystick.Joystick(0)
joystick.init()

# Initialize the serial connection
serial_port = 'COM3'  # Replace with your actual COM port
baud_rate = 115200
ser = serial.Serial(serial_port, baud_rate, timeout=1)

def get_joystick_data():
    # Handle joystick events
    pygame.event.pump()

    # Get values from the joystick axes
    axis1 = joystick.get_axis(0)  # Left-right axis of the first stick
    axis2 = joystick.get_axis(1)  # Up-down axis of the first stick

    # Convert axis values from range [-1, 1] to [0, 1023]
    sensor_value1 = int((axis1 + 1) * 511.5)  # Map to 0-1023
    sensor_value2 = int((axis2 + 1) * 511.5)  # Map to 0-1023

    # Get the state of a button (button 0 in this case)
    button_state = joystick.get_button(0)  # Boolean (pressed or not)

    return sensor_value1, sensor_value2, button_state

try:
    while True:
        # Get joystick data
        sensor_value1, sensor_value2, button_state = get_joystick_data()

        # Send the values to the ESP8266 via serial
        data_to_send = f"{sensor_value1} {sensor_value2} {button_state}\n"
        ser.write(data_to_send.encode())

        # Print sent values for debugging
        print(f"Sent: {data_to_send.strip()}")

        # Wait before the next iteration
        time.sleep(0.1)  # Adjust delay to control sending frequency

except KeyboardInterrupt:
    print("Program interrupted by user")

finally:
    # Close the serial port and quit pygame
    ser.close()
    pygame.quit()
