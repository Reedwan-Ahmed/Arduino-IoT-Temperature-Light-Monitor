import serial
import time
import requests

SERIAL_PORT = 'COM3'
BAUD_RATE = 9600

THINGSPEAK_API_KEY = "55F9J4KF3PTZGO3Y"
THINGSPEAK_URL = "https://api.thingspeak.com/update"

def upload_to_thingspeak(light, temp):
    payload = {'api_key': THINGSPEAK_API_KEY, 'field1': light, 'field2': temp}
    try:
        response = requests.post(THINGSPEAK_URL, data=payload)
        if response.status_code == 200:
            print("Data uploaded successfully")
        else:
            print(f"Failed to upload data. Status code: {response.status_code}")
    except Exception as e:
        print(f"Error uploading data: {e}")

def main():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Connected to {SERIAL_PORT} at {BAUD_RATE} baud.")
        time.sleep(2)  # Arduino reset delay

        last_upload_time = 0
        latest_light = None
        latest_temp = None

        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8').strip()
                if line:
                    print(f"Received: {line}")
                    if "LDR:" in line and "TEMP:" in line:
                        try:
                            parts = line.split(',')
                            light_str = parts[0].split(":")[1]
                            temp_str = parts[1].split(":")[1]

                            latest_light = int(light_str)
                            latest_temp = float(temp_str)

                            print(f"Parsed - Light: {latest_light}, Temp: {latest_temp}")

                        except Exception as e:
                            print(f"Error parsing line: {e}")

            # Upload only every 15 seconds
            current_time = time.time()
            if latest_light is not None and latest_temp is not None:
                if current_time - last_upload_time >= 15:
                    upload_to_thingspeak(latest_light, latest_temp)
                    last_upload_time = current_time

            time.sleep(0.1)  # short delay for responsive reading

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except KeyboardInterrupt:
        print("Program stopped by user.")

if __name__ == "__main__":
    main()
