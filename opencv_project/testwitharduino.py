import cv2
import mediapipe as mp
import requests
import time

# Setup for MediaPipe Hands and Drawing utils
mp_drawing = mp.solutions.drawing_utils
mp_hands = mp.solutions.hands

URL = "http://192.168.4.1/POS?x={}&y={}"  # Use your Arduino's IP here

smooth_x, smooth_y = 90, 90
alpha = 0.2  # smoothing factor

# Simple exponential moving average for smoothing
def moving_average(prev, new, alpha=0.2):
    return int(prev * (1 - alpha) + new * alpha)

# Start capturing from webcam
cap = cv2.VideoCapture(0)
with mp_hands.Hands(min_detection_confidence=0.7, min_tracking_confidence=0.5) as hands: # Track laser state to avoid repeating the same siganl
    laser_on = False
    while cap.isOpened():
        success, image = cap.read()
        if not success:
            continue

        image = cv2.flip(image, 1) # Flip image for mirror view
        image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        results = hands.process(image_rgb) # Run hand tracking
        image = cv2.cvtColor(image_rgb, cv2.COLOR_RGB2BGR)

        if results.multi_hand_landmarks:
            for hand_landmarks in results.multi_hand_landmarks:
                mp_drawing.draw_landmarks(image, hand_landmarks, mp_hands.HAND_CONNECTIONS)
                # Draw hand connections on screen
                index_tip = hand_landmarks.landmark[8]
                thumb_tip = hand_landmarks.landmark[4]
                #Get landmarks for index fingertip and thumb tip
                h, w, _ = image.shape
                index_x = int(index_tip.x * w)
                index_y = int(index_tip.y * h)

                # Mirror movement
                mapped_x = 180 - int(index_tip.x * 180)
                mapped_y = int(index_tip.y * 180)

                smooth_x = moving_average(smooth_x, mapped_x, alpha)
                smooth_y = moving_average(smooth_y, mapped_y, alpha)
                # Calculate distance between thumb and index finger (pinch detection)
                pinch_dist = ((index_tip.x - thumb_tip.x) ** 2 + (index_tip.y - thumb_tip.y) ** 2) ** 0.5

                try:
                    if pinch_dist < 0.05: # Pinch detected -> turn laser on
                        if not laser_on:
                            laser_on = True
                            print("Laser ON")
                        requests.get(URL.format(999, 999)) # Send trigger to Arduino
                    else:
                        if laser_on:
                            laser_on = False
                            print("Laser OFF")
                        requests.get(URL.format(smooth_x, smooth_y))
                except Exception as e:
                    print("Connection error:", e) #Hanlde connections issues

        cv2.imshow('Hand Tracker', image)
        if cv2.waitKey(1) & 0xFF == 27:
            break # Exit if ESC is pressed
 #Release resources
cap.release()
cv2.destroyAllWindows()
