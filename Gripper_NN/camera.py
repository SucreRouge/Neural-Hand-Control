import numpy as np
import cv2
flag=0

def click(event, x, y, flags, param):
    global flag
    
    if event == cv2.EVENT_LBUTTONDOWN:
        Coordfile.write(repr(x)+','+repr(y)+','+'\n')
        print(x,y)
        flag=1

cv2.namedWindow('Camera',cv2.WINDOW_AUTOSIZE)
cv2.setMouseCallback('Camera', click)
cap=cv2.VideoCapture(1)
Coordfile=open('InputCoord.csv','w');
while(True):
    ret, frame=cap.read()
    cv2.imshow('Camera',frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    if flag==1:
        break
Coordfile.close()
cap.release()
cv2.destroyAllWindows
