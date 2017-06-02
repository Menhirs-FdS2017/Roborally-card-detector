#!/bin/python2
# -*- coding: utf-8 -*-
# USAGE
# python recognize_digits.py

# import the necessary packages
from imutils.perspective import four_point_transform
from imutils import contours
import imutils
import cv2
import numpy as np
import time

blurConstant = (3,3)

def looksLikeRectangle(shape, delta=15):
	ax,ay = shape[0][0][0], shape[0][0][1]
	bx,by = shape[1][0][0], shape[1][0][1]
	cx,cy = shape[2][0][0], shape[2][0][1]
	dx,dy = shape[3][0][0], shape[3][0][1]

	#print (ax,ay), (bx,by), (cx,cy), (dx,dy)
	x1 = abs(ax - bx) < delta and abs(cx - dx) < delta
	x2 = abs(ax - cx) < delta and abs(bx - dx) < delta
	x3 = abs(ax - dx) < delta and abs(bx - cx) < delta

	y1 = abs(ay - by) < delta and abs(cy - dy) < delta
	y2 = abs(ay - cy) < delta and abs(by - dy) < delta
	y3 = abs(ay - dy) < delta and abs(by - cy) < delta
	return (x1 or x2 or x3) and (y1 or y2 or y3)

def isInTop(shape, height, portion=.25):
        for p in shape:
                if p[0][1] > height * portion:
                        return False
        return True



# define the dictionary of digit segments so we can identify
# each digit on the thermostat
DIGITS_LOOKUP = {
	(1, 1, 1, 0, 1, 1, 1): 0,
	(0, 0, 1, 0, 0, 1, 0): 1,
	(1, 0, 1, 1, 1, 1, 0): 2,
	(1, 0, 1, 1, 0, 1, 1): 3,
	(0, 1, 1, 1, 0, 1, 0): 4,
	(1, 1, 0, 1, 0, 1, 1): 5,
	(1, 1, 0, 1, 1, 1, 1): 6,
	(1, 0, 1, 0, 0, 1, 0): 7,
	(1, 1, 1, 1, 1, 1, 1): 8,
	(1, 1, 1, 1, 0, 1, 1): 9
}

# load the example image
# image = cv2.imread('example.jpg')
# image = cv2.imread("/home/gus3000/D_DRIVE/Images/Opencv/D_Side.jpg")
cap = cv2.VideoCapture(0)
#cap.set(cv2.CAP_PROP_AUTOFOCUS, 0) # turn the autofocus off
lastGoodFrame = np.zeros((256, 256, 1), dtype = "uint8")
warped = np.zeros((256, 256, 1), dtype = "uint8")
cardAlone = np.zeros((256, 256, 1), dtype = "uint8")
while(True):
    # Capture frame-by-frame
    ret, frame = cap.read()

    # Our operations on the frame come here
    # gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # Display the resulting frame
    cv2.imshow('frame',frame)
    res = cv2.waitKey(1) & 0xFF
    if res == ord('q'):
        cap.release()
        cv2.destroyAllWindows()
        break

    elif res == 255:
        
        # When everything done, release the capture

        image = frame
        # pre-process the image by resizing it, converting it to
        # graycale, blurring it, and computing an edge map
        image = imutils.resize(image, height=500)
        cv2.imshow("image", image)
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        cv2.imshow("gray", gray)
        blurred = cv2.GaussianBlur(gray, blurConstant, 0)
        cv2.imshow("blurred", blurred)
        edged = cv2.Canny(blurred, 50, 200, 255)
        #edged = cv2.Canny(gray, 50, 200, 255)
        cv2.imshow("edged", edged)
        
        # find contours in the edge map, then sort them by their
        # size in descending order
        cnts = cv2.findContours(edged.copy(), cv2.RETR_EXTERNAL,
	                        cv2.CHAIN_APPROX_SIMPLE)
        cnts = cnts[0] if imutils.is_cv2() else cnts[1]
        cnts = sorted(cnts, key=cv2.contourArea, reverse=True)
        displayCnt = None
        
        # loop over the contours
        for c in cnts:
	        # approximate the contour
	        peri = cv2.arcLength(c, True)
	        approx = cv2.approxPolyDP(c, 0.02 * peri, True)
                
	        # if the contour has four vertices, then we have found
	        # the thermostat display
	        if len(approx) == 4:
 		        displayCnt = approx
		        break
                
        # extract the thermostat display, apply a perspective transform to it
        if(displayCnt is not None):
            warped = four_point_transform(gray, displayCnt.reshape(4, 2))
            cardAlone = four_point_transform(image, displayCnt.reshape(4, 2))

        if np.size(warped,0) < np.size(warped,1): #if height < width
                #rotate 90°
                warped = cv2.flip(cv2.transpose(warped),1)
                
                cardAlone = cv2.flip(cv2.transpose(cardAlone),1)
                
        cv2.imshow("warped", warped)
        #cv2.waitKey(0)
        cv2.imshow("cardAlone", cardAlone)
        #cv2.waitKey(0)
        
        # Ici, cardAlone contient l'image croppée et redressée :D
        
        blurred = cv2.GaussianBlur(warped, blurConstant, 0)
        # cv2.imshow("blurred alone", blurred)
        # cv2.waitKey(0)
        #edged = cv2.Canny(blurred, 50, 200, 255)
        edged = cv2.Canny(warped, 50, 200, 3) #not 255, you fool !
        #edged = four_point_transform(edged, displayCnt.reshape(4,2))
        cv2.imshow("edged alone", edged)
        # cv2.waitKey(0)
        
        
        # find contours in the edge map, then sort them by their
        # size in descending order
        cnts = cv2.findContours(edged.copy(), cv2.RETR_EXTERNAL,
	                        cv2.CHAIN_APPROX_SIMPLE)
        cnts = cnts[0] if imutils.is_cv2() else cnts[1]
        cnts = sorted(cnts, key=cv2.contourArea, reverse=True)
        displayCnt = []
        
        # loop over the contours
        count = 0
        for c in cnts:
                # approximate the contour
	        peri = cv2.arcLength(c, True)
	        approx = cv2.approxPolyDP(c, 0.02 * peri, True)
                
	        # if the contour looks like a rectangle, we did it !
	        delta = 10
	        if len(approx) == 4 and looksLikeRectangle(approx) and isInTop(approx,np.size(cardAlone,0)) and peri > 80 and peri < 150:
                        displayCnt.append(approx)
                        """x1,y1 = approx[0][0][0], approx[0][0][1]
                        x2,y2 = approx[2][0][0], approx[2][0][1]
                        cv2.rectangle(cardAlone, (x1,y1), (x2,y2),(0,255,0), 1)"""
                        cv2.imshow("cardAlone", cardAlone)
                        count += 1
                        
        if(count == 2):
            lastGoodFrame = cardAlone
            cap.release()
            cv2.destroyAllWindows()
            break;
                        
        time.sleep(0.1)
        
cv2.imshow("Last Good Frame", lastGoodFrame)
arrowRectangle = four_point_transform(cardAlone, displayCnt[1].reshape(4, 2))
digitRectangle = four_point_transform(cardAlone, displayCnt[0].reshape(4, 2))
digitRectangleGray = cv2.cvtColor(digitRectangle, cv2.COLOR_BGR2GRAY);
cv2.imshow("digitRectangleGray", digitRectangle)
# find contours in the thresholded image, then initialize the
# digit contours lists
cnts = cv2.findContours(digitRectangleGray.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
cnts = cnts[0] if imutils.is_cv2() else cnts[1]
digitCnts = []
 
# loop over the digit area candidates
"""for c in cnts:
	# compute the bounding box of the contour
	(x, y, w, h) = cv2.boundingRect(c)
 
	# if the contour is sufficiently large, it must be a digit
	if w >= 15 and (h >= 5 and h <= 50):
		digitCnts.append(c)

print(len(digitCnts))

digitCnts = contours.sort_contours(digitCnts, method="left-to-right")[0]
digits = []

for c in digitCnts:
	# extract the digit ROI
	(x, y, w, h) = cv2.boundingRect(c)
	roi = digitRectangleGray[y:y + h, x:x + w]
 
	# compute the width and height of each of the 7 segments
	# we are going to examine
	(roiH, roiW) = roi.shape
	(dW, dH) = (int(roiW * 0.25), int(roiH * 0.15))
	dHC = int(roiH * 0.05)
 
	# define the set of 7 segments
	segments = [
		((0, 0), (w, dH)),	# top
		((0, 0), (dW, h // 2)),	# top-left
		((w - dW, 0), (w, h // 2)),	# top-right
		((0, (h // 2) - dHC) , (w, (h // 2) + dHC)), # center
		((0, h // 2), (dW, h)),	# bottom-left
		((w - dW, h // 2), (w, h)),	# bottom-right
		((0, h - dH), (w, h))	# bottom
	]
	on = [0] * len(segments)
	# loop over the segments
	for (i, ((xA, yA), (xB, yB))) in enumerate(segments):
		# extract the segment ROI, count the total number of
		# thresholded pixels in the segment, and then compute
		# the area of the segment
		segROI = roi[yA:yB, xA:xB]
		total = cv2.countNonZero(segROI)
		area = (xB - xA) * (yB - yA)
 
		# if the total number of non-zero pixels is greater than
		# 50% of the area, mark the segment as "on"
		if total / float(area) > 0.5:
			on[i]= 1
 
	# lookup the digit and draw it on the image
	digit = DIGITS_LOOKUP[tuple(on)]
	digits.append(digit)
	cv2.rectangle(output, (x, y), (x + w, y + h), (0, 255, 0), 1)
	cv2.putText(output, str(digit), (x - 10, y - 10),
		cv2.FONT_HERSHEY_SIMPLEX, 0.65, (0, 255, 0), 2)
        
# display the digits
print(u"{}{}.{} \u00b0C".format(*digits))
cv2.imshow("Input", image)
cv2.imshow("Output", output)"""
cv2.waitKey(0)
