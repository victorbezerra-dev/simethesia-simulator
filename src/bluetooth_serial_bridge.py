import serial
import serial.tools.list_ports
import threading
import time

def find_port(keyword):
    """Searches for a serial port matching a given keyword in its description or name."""
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if keyword.lower() in port.description.lower() or keyword.lower() in port.device.lower():
            return port.device
    return None

def bridge_serial(src, dst, src_name, dst_name):
    """Continuously reads data from one serial port and writes it to another."""
    while True:
        try:
            if src.in_waiting:
                line = src.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(f"[{src_name} → {dst_name}] {line}")
                    dst.write((line + '\n').encode('utf-8'))
        except Exception as e:
            print(f"[ERROR] Failed to bridge {src_name} to {dst_name}: {e}")
        time.sleep(0.01)

def main():
    print("Scanning for available serial ports...")

    bt_port = find_port("COM6") or input("Bluetooth port not found. Please enter manually (e.g., COM7): ")
    arduino_port = find_port("arduino") or input("Arduino port not found. Please enter manually (e.g., COM10): ")

    print(f"Bluetooth found on: {bt_port}")
    print(f"Arduino found on: {arduino_port}")

    try:
        bluetooth = serial.Serial(bt_port, 115200, timeout=0.1)
        arduino = serial.Serial(arduino_port, 115200, timeout=0.1)
    except Exception as e:
        print("Failed to open serial ports:", e)
        return

    print("\nSerial bridge running. Press Ctrl+C to stop.\n")

    # Start threads for bidirectional communication
    t_bt_to_arduino = threading.Thread(target=bridge_serial, args=(bluetooth, arduino, "APP", "Arduino"))
    t_arduino_to_bt = threading.Thread(target=bridge_serial, args=(arduino, bluetooth, "Arduino", "APP"))

    t_bt_to_arduino.daemon = True
    t_arduino_to_bt.daemon = True

    t_bt_to_arduino.start()
    t_arduino_to_bt.start()

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nShutting down bridge...")
        bluetooth.close()
        arduino.close()

if __name__ == "__main__":
    main()