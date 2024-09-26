import cv2
from deepface import DeepFace
import requests
import numpy as np

# Load face cascade classifier
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

# Define the coordinates of the square
square_x1, square_y1 = 300, 200  # Top-left corner coordinates
square_x2, square_y2 = square_x1 + 250, square_y1 + 180  # Bottom-right corner coordinates
server_ip = "192.168.4.1"  # Replace with the IP address of your Arduino server
esp32_cam_url = "http://192.168.4.5/cam-hi.jpg"  # Replace with your ESP32-CAM IP address and desired resolution endpoint

def get_frame():
    resp = requests.get(esp32_cam_url, stream=True)
    if resp.status_code == 200:
        arr = np.asarray(bytearray(resp.raw.read()), dtype=np.uint8)
        frame = cv2.imdecode(arr, -1)
        return frame
    else:
        print("Failed to get frame from ESP32-CAM")
        return None

while True:
    frame = get_frame()
    if frame is None:
        continue

    try:
        # Convert frame to grayscale
        gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        # Convert grayscale frame to RGB format
        rgb_frame = cv2.cvtColor(gray_frame, cv2.COLOR_GRAY2RGB)

        # Detect faces in the frame
        faces = face_cascade.detectMultiScale(gray_frame, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))

        for (x, y, w, h) in faces:
            # Extract the face ROI (Region of Interest)
            face_roi = rgb_frame[y:y + h, x:x + w]

            # Perform emotion analysis on the face ROI
            try:
                result = DeepFace.analyze(face_roi, actions=['emotion'], enforce_detection=False)
            except Exception as e:
                print(f"Error in emotion analysis: {e}")
                continue

            # Determine the dominant emotion
            emotion = result[0]['dominant_emotion']
            print(emotion)

            # Map emotions to values
            emotion_values = {
                "happy": 1,
                "angry": 2,
                "sad": 3,
                "fear": 4,
                "surprise": 5,
                "neutral": 6
            }

            total = emotion_values.get(emotion, 0)

            # Draw rectangle around face and label with predicted emotion
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 0, 255), 2)
            cv2.putText(frame, emotion, (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0, 0, 255), 2)
            cv2.rectangle(frame, (square_x1, square_y1), (square_x2, square_y2), (155, 0, 0), 2)
            cv2.putText(frame, "Place your hand here", (square_x1, square_y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 1)

            # For the sake of example, assume hand_in_square detection logic is a placeholder
            hand_in_square = True  # Replace with actual hand detection logic
            try:
                if hand_in_square:
                    requests.get(f"http://{server_ip}/data/?sensor_reading={{\"sensor0_reading\":{total}}}")
                else:
                    total = 0  # If no hand is detected, set total to 0
                    requests.get(f"http://{server_ip}/data/?sensor_reading={{\"sensor0_reading\":{total}}}")
            except requests.RequestException as e:
                print(f"Error in sending request: {e}")

    except Exception as e:
        print(f"Error in processing frame: {e}")

    # Display the resulting frame
    cv2.imshow('Real-time Emotion Detection', frame)

    # Press 'q' to exit
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release the capture and close all windows
cv2.destroyAllWindows()
