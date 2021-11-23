import cv2
import numpy as np
from matplotlib import pyplot as plt

def main():

	#receive the image
	#figure out what step it's on. if it's first time receiving image, always start with step 1
	firstRecv = True #make false after first attempt

	#use boolean flags to keep track of what part of path you're in. if something's false, backtrack to last true
	failed = False #set true if can't find anything and restart
	
	searchOnscreen = False
	settingsOnscreen = False
	#probably gonna take the type it right in route for now
	settingsClickableOnscreen = False #able to click on settings. start checking after typing, should go after "windows se"
	secSettingsOnscreen = False #security window is onscreen
	secWindowExists = False #if accidentally minimizes window, check that it's still there
	vtProtectClickableOnscreen = False #can go to the virus and threat protection settings
	vtOnscreen = False #we are where we need to go
	manageSettingsOnscreen = False
	#SCROLL UNTIL REACHING TAMPER PROTECTION FOR BOTH NECESSITY AND CERTAIN BYPASSING
	tamperProtOnscreen = False #loop while scrolling until this is true
	toggleOnscreen = False #little toggle button
	uacOnscreen = False #likely won't matter anyway
	tpOffCheck = False #make sure it worked
	#scroll up now to real time protection
	rtProtOnscreen = False #shut real time protection
	rtOffCheck = False #ensure real time protection is off
	cloudOnscreen = False
	cloudOffCheck = False
	sampleOnscreen = False
	sampleOffCheck = False
	finished = False

	attemptAgain = True
	winExistsCheck = True
	attempts = 0
	verifiedSearch = False
	verifiedSettings = False
	verifiedVirusThreatProt = False
	verifiedVTProtOptions = False
	verifiedManageSettings = False
	verifiedTamperProt = False
	verifiedVTProtSettings = False
	togglerFound = False

	#PUT THEM ALL INTO LIST SO ALL CAN BE CHANGED TO FALSE IN EVENT OF RESET
	flags = [searchOnscreen, settingsClickableOnscreen,
			secSettingsOnscreen, secWindowExists, vtProtectClickableOnscreen,
			vtOnscreen, manageSettingsOnscreen, tamperProtOnscreen, toggleOnscreen,
			uacOnscreen, tpOffCheck, rtProtOnscreen, rtOffCheck, cloudOnscreen,
			cloudOffCheck, sampleOnscreen, sampleOffCheck, attemptAgain, winExistsCheck, verifiedSearch,
			verifiedSettings, verifiedVirusThreatProt, verifiedVTProtOptions, verifiedManageSettings,
			verifiedTamperProt, verifiedVTProtSettings, togglerFound]


	#put these above the list and add to list after created
	#may need to make some or all of these global/put them outside the main

	#IN C --> IF STATUS > 0, CLICK.

	scrollDown = True #scroll upward when false; -9 is scroll down and -8 is scroll up
	
	#loop through and perform appropriate actions as needed - make sure everything is reset to false as needed if can't find
	while(not finished and attempts < 10):
		#method should wait on successful reception of image
		received = 'desktop.bmp' #should run the method to receive every time and wait on that
		if(firstRecv or failed): #if first time or need to restart
			attempts += 1
			failed = False #in case not already
			target = 'search.jpg'
			coords = findImg(received, target)
			if(coords[0] != -1): #successfully found
				searchOnscreen = True #move to next step
				sendStatus(1) #first step completed
				sendCoordinates(coords)
			else: #ADD EDGE CASE - IF NOT THERE KEYPRESS F11 AND ESC IN CASE OF FULLSCREEN THEN TRY AGAIN
				failed = True
				sendStatus(-1) #failure
				sendCoordinates(coords) #will be (-1,-1) and disregarded by program
			firstRecv = False
		elif(searchOnscreen and not verifiedSearch): #haven't already succeeded, but made it this far
			received = '1.jpg'
			target = 'searchsuccesstypehere.jpg'
			coords = findImg(received, target)
			if(coords[0] != -1):
				verifiedSearch = True
				sendStatus(2) #able to search now
				sendCoordinates(coords)
			else:
				attemptAgain = standardCheck(coords)[1]
		#able to search, so should have searched at this point and sent with clickable
		elif(verifiedSearch and not verifiedSettings):
			received = '2.jpg'
			target = 'winsecoption.jpg'
			coords = findImg(received, target)
			if(coords[0] != -1):
				verifiedSettings = True
				sendStatus(2) #has successfully made it to settings, it should open when clicked
				sendCoordinates(coords)
			else:
				target = 'winsecapp.jpg'
				coords = findImg(received, target)
				if(coords[0] != -1):
					verifiedSettings = True
					sendStatus(3) #has successfully made it to settings, it should open when clicked
					sendCoordinates(coords)
				else:
					target = 'winsecappicon.jpg'
					coords = findImg(received, target)
					if(coords[0] != -1):
						verifiedSettings = True
						sendStatus(2) #has successfully made it to settings, it should open when clicked
						sendCoordinates(coords)
					else:
						attemptAgain = standardCheck(coords)
		elif(verifiedSettings and not verifiedVirusThreatProt):
			received = '3.jpg'
			target = 'virusthreatprotectionglance.jpg'
			coords = findImg(received, target)
			if(coords[0] != -1):
				verifiedVirusThreatProt = True
				sendStatus(4) #has successfully made it to vtprot
				sendCoordinates(coords)
				winExistsCheck = True #in case this is the second attempt
			else:
				#check for other possible icons
				target = 'virusthreatprotectionfull.jpg'
				coords = findImg(received, target)
				if(coords[0] != -1):
					verifiedVirusThreatProt = True
					sendStatus(4) #has successfully made it to vtprot
					sendCoordinates(coords)
					winExistsCheck = True #in case this is the second attempt
					continue
				else:
					target = 'virusthreatprotectionicon.jpg'
					coords = findImg(received, target)
					if(coords[0] != -1):
						verifiedVirusThreatProt = True
						sendStatus(4) #has successfully made it to vtprot
						sendCoordinates(coords)
						winExistsCheck = True #in case this is the second attempt
						continue
				#check if minimized
				target = 'winsecicon.jpg'
				update = existsCheck(received, target)
				winExistsCheck = update[1]
				attemptAgain = update[0]
		elif(verifiedVirusThreatProt and not verifiedVTProtOptions): #if successful, it's time to scroll, status should reflect that
			received = '4.jpg'
			target = 'virusthreatprotectionwindow.jpg'
			coords = findImg(received, target)
			if(coords[0] != -1):
				verifiedVTProtOptions = True
				sendStatus(5) #has successfully made it to vtprotoptions
				sendCoordinates(coords) #irrelevant here
				winExistsCheck = True #in case this is the second attempt
			#check if minimized
			else:
				target = 'winsecicon.jpg'
				update = existsCheck(received, target)
				winExistsCheck = update[1]
				attemptAgain = update[0]
		elif(verifiedVTProtOptions and not verifiedManageSettings): #SCROLL CHECK
			received = '5.jpg'
			target = 'managesettings.jpg'
			coords = findImg(received, target)
			if(coords[0] != -1):
				verifiedManageSettings = True
				sendStatus(6) #has made it to manage settings
				sendCoordinates(coords)
				winExistsCheck = True #in case this is the second attempt
			else: #need to differentiate between need to scroll and having issue
				target = 'virusthreatprotectionglance.jpg'
				coords = findImg(received, target)
				if(coords[0] != -1):
					sendStatus(-9) #need to scroll downward
					sendCoordinates(coords) #irrelevant here
					continue
				else:
					target = 'winsecicon.jpg'
					update = existsCheck(received, target)
					winExistsCheck = update[1]
					attemptAgain = update[0]
		elif(verifiedManageSettings and not verifiedVTProtSettings):
			received = '6.jpg'
			target = 'virusthreatprotectionsettings.jpg'
			coords = findImg(received, target)
			if(coords[0] != -1):
				verifiedVTProtSettings = True
				sendStatus(7) #has successfully made it to settings, definitely in right place
				sendCoordinates(coords) #irrelevant here
				winExistsCheck = True #in case this is the second attempt
			else:
				#first check for icon in case text changed
				#we still start with full check because it's most accurate, doesn't risk similar icons
				target = 'vtproticon.jpg'
				coords = findImg(received, target)
				if(coords[0] != -1):
					verifiedVTProtSettings = True
					sendStatus(7) #has successfully made it to settings, definitely in right place
					sendCoordinates(coords) #irrelevant here
					winExistsCheck = True #in case this is the second attempt
				else:
					#first check to see if in the process of looking for it after returning from following step
					if(not scrollDown): #currently trying to verify correct place
						sendStatus(-8) #tell program to scroll up then send again
						sendCoordinates(coords) #irrelevant here
						winExistsCheck = True
					else:
						target = 'winsecicon.jpg'
						update = existsCheck(received, target)
						winExistsCheck = update[1]
						attemptAgain = update[0]
		#in the right place, now scroll until it's found
		elif(verifiedVTProtSettings and not verifiedTamperProt):
			received = '7.jpg'
			target = 'tamperfull.jpg'
			coords = findImg(received, target)
			if(coords[0] != -1):
				verifiedTamperProt = True
				sendStatus(8) #has successfully made it to tamper protection, specifically entire part including toggle button
				sendCoordinates(coords) #irrelevant here
				winExistsCheck = True #in case this is the second attempt
			else: 
				#check for part of tp. if exists, verify it's there with variable then scroll
				target = 'tamperheader.jpg'
				coords = findImg(received, target)
				if(coords[0] != -1):
					if(scrollDown):
						sendStatus(-9) #scroll down
					else:
						sendStatus(-8) #scroll up
					sendCoordinates(coords) #irrelevant
				#if not, check for all other options. if at least one exists, then scroll downward
				else:
					check = False
					images_list = ['realtimeheader.jpg', 'cloudheader.jpg', 'autosampleheader.jpg']
					for item in images_list:
						tempcoords = findImg(received, item)
						if(tempcoords[0] != -1):
							check = True
					if(check == true):
						if(scrollDown):
							sendStatus(-9)
						else:
							sendStatus(-8)
					#if not, there is an issue (or scrolled too far, this shouldn't be possible!!!)
					else:
					#just in case, check for bottom of page
						target = 'pageend.jpg'
						coords = findImg(received, target)
						if(tempcoords[0] != -1):
							#if true, change status so scrolling happens UPWARD instead
							if(scrollDown == True):
								scrollDown = False
							#set verifiedvtprotsettings to false (or have second checker.)
								verifiedvtprotsettings = False
							#if set to scroll upward and eventually verifiedvtprotsettings is true, then consider failure
							else:
								target = 'winsecicon.jpg'
								update = existsCheck(received, target)
								winExistsCheck = update[1]
								attemptAgain = update[0]
							#if false, do minimize check/failure check
				#and a part of tp.
				#if ALL are false, there is an issue (or scrolled too far but should NOT be possible to get to this point) which should trigger minimize check
				#if window exists, make verifiedVTProtSettings false and run from there, either is in wrong spot or may be a failure
				#need to differentiate between need to scroll and having issue
				target = 'virusthreatprotectionglance.jpg'
				coords = findImg(received, target)
				if(coords[0] != -1):
					sendStatus(-9) #need to scroll downward
					sendCoordinates(coords) #irrelevant here
					continue
		#CHECK FOR TOGGLE BUTTON BEFORE MOVING ON, AND CROP IMAGE AND ONLY LOOK FOR THE CORRECT TOGGLE BUTTON
		elif(verifiedTamperProt and not togglerFound):
			received = '7.jpg'
			target = 'tamperfull.jpg'
			img = cropImg(received, target)
			target = 'toggleonicon.jpg'
			coords = findImg(img, target)
			if(coords[0] != -1):
				finished = True #for testing
				togglerFound = True
				sendStatus(9)
				sendCoordinates(coords)
				winExistsCheck = True
			else:
				target = 'winsecicon.jpg'
				update = existsCheck(received, target)
				winExistsCheck = update[1]
				attemptAgain = update[0]





		#might have been minimized, otherwise hasn't been clicked on
		#elif(searchOnscreen and !)
	
		#check after for attempts num to see if succeeded or failed

	received = 'desktop.bmp'
	target = 'search.jpg'
	print(findImg(received, target))

def standardCheck(coords, attemptAgain):
	if(attemptAgain):
		sendStatus(-2) #failed, send another screenshot
		sendCoordinates(coords) #(-1,-1)
		attemptAgain = False
	else:
		failed = True
		attemptAgain = True #try again next time
		resetFlags()
	return attemptAgain

def existsCheck(received, target, winExistsCheck, attemptAgain):
	coords = findImg(received, target)
	if(coords[0] != -1 and winExistsCheck):
		sendStatus(-3) #may be minimized or obstructed, click it then send screenshot again
		sendCoordinates(coords)
		winExistsCheck = False #only test this once
	else:
		attemptAgain = standardCheck(coords, attemptAgain)[1]
	return (attemptAgain, winExistsCheck)

def cropImg(i, t):
	image = cv2.imread(i) #read in image
	#cv2.imshow('Rainforest', image) #display the image - delete later
	#cv2.waitKey(0) #wait for any key to be pressed - delete later
	gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY) #make grayscale so it's easier to work on

	template= cv2.imread(t,0)


	result= cv2.matchTemplate(gray, template, cv2.TM_CCOEFF_NORMED) #find the image within the larger image - cv2.TM_CCOEFF
	threshold = .70

	print(1 - result)

	loc = np.where(result >= threshold)
	print(loc)

	if(len(loc[1]) != 0):

		print("Found!")

		#location stuff:
		min_val, max_val, min_loc, max_loc= cv2.minMaxLoc(result)

		print(min_val)
		print(max_val)
		print(min_loc)
		print(max_loc)

		height, width= template.shape[:2]
		h, w = image.shape[:2]

		print()
		print(h)
		print(w)
		print()

		top_left= max_loc
		bottom_right= (top_left[0] + width, top_left[1] + height)

		print('top left')
		print(top_left)
		print('bottom right')
		print(bottom_right)
		
		# cv2.rectangle(image, top_left, bottom_right, (0,0,255), 5)
		#cropped_img = image[bottom_right[1]:top_left[1], top_left[0]:bottom_right[0]] #may need to be flipped?
		cropped_img = image[top_left[1]:bottom_right[1], top_left[0]:bottom_right[0]]
		# cv2.circle(image, (midpoint[0], midpoint[1]), 10, (0,0,255), -1)

		# cv2.imshow('Desktop', image)
		cv2.namedWindow('finalCImg', cv2.WINDOW_NORMAL)
		cv2.resizeWindow('finalCImg', 540, 540)
		cv2.imshow("finalCImg",cropped_img)
		cv2.waitKey(0)
		cv2.destroyAllWindows()
	else:
		print("Not Found.")

	return cropped_img

	
def findImg(i, t):
	if(isinstance(i, str)):
		image = cv2.imread(i) #read in image
	else:
		image = i
	#cv2.imshow('Rainforest', image) #display the image - delete later
	#cv2.waitKey(0) #wait for any key to be pressed - delete later
	gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY) #make grayscale so it's easier to work on

	template= cv2.imread(t,0)


	result= cv2.matchTemplate(gray, template, cv2.TM_CCOEFF_NORMED) #find the image within the larger image - cv2.TM_CCOEFF
	threshold = .70

	print(1 - result)

	loc = np.where(result >= threshold)
	print(loc)

	if(len(loc[1]) != 0):

		print("Found!")

		#location stuff:
		min_val, max_val, min_loc, max_loc= cv2.minMaxLoc(result)

		print(min_val)
		print(max_val)
		print(min_loc)
		print(max_loc)

		height, width= template.shape[:2]
		h, w = image.shape[:2]

		print()
		print(h)
		print(w)
		print()

		top_left= max_loc
		bottom_right= (top_left[0] + width, top_left[1] + height)
		
		midpoint = [-1,-1]
		midpoint[0] = int((top_left[0] + bottom_right[0]) / 2)
		midpoint[1] = int((top_left[1] + bottom_right[1]) / 2)
		midpoint = tuple(midpoint)
		print(midpoint)

		x_percentage = midpoint[0] / w
		y_percentage = midpoint[1] / h

		print('x% = ' + str(x_percentage) + '%, y% = ' + str(y_percentage) + '%')

		actualx = int(x_percentage * w)
		actualy = int(y_percentage * h)

		print(str(actualx) + ', ' + str(actualy))

		cv2.rectangle(image, top_left, bottom_right, (0,0,255), 5)
		cv2.circle(image, (midpoint[0], midpoint[1]), 10, (0,0,255), -1)

		# cv2.imshow('Desktop', image)
		cv2.namedWindow('final_Img', cv2.WINDOW_NORMAL)
		cv2.resizeWindow('final_Img', 1080, 540)
		cv2.imshow("final_Img",image)
		cv2.waitKey(0)
		cv2.destroyAllWindows()
	else:
		print("Not Found.")
		x_percentage = -1
		y_percentage = -1

	coords = (x_percentage, y_percentage) #currently given in percents
	return coords

def sendCoordinates(coords):
	print(coords)
	return 0

def sendStatus(status):
	return 0

def resetFlags(flags):
	for flag in flags:
		flag = False

if __name__ == "__main__":
    main()