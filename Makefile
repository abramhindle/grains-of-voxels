CSOUND=/usr/bin/csound -dm6 -+rtaudio=alsa -dm6 -o devaudio -L stdin
CSOUNDJACK=/usr/bin/csound -dm6 -+rtaudio=jack -dm6 -o devaudio -L stdin -B 2048
OPENCV=-lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_gpu -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_objdetect -lopencv_video 
SHAREDFLAGS=-DSDL=1 -lGL -lglut -lSDL -lfreenect -lm `pkg-config --cflags sdl` `pkg-config --cflags libfreenect` `pkg-config --libs sdl` `pkg-config --libs libfreenect` `pkg-config --cflags opencv` `pkg-config --libs opencv` -lpthread -llo
CC=gcc -std=c99 -O3 
CPP=g++ -O3 -DSDL=1 -lGL -lglut -lSDL  ${SHAREDFLAGS} 


KinectCVShapeScan:	KinectCVShapeScan.cpp
	$(CPP) -I/usr/include/oscpack/ KinectCVShapeScan.cpp -o KinectCVShapeScan $(SHAREDFLAGS) -loscpack

KinectCVShapeMultiScan:	KinectCVShapeMultiScan.cpp
	$(CPP) -I/usr/include/oscpack/ KinectCVShapeMultiScan.cpp -o KinectCVShapeMultiScan $(SHAREDFLAGS) -loscpack

KinectCVShapeSample:	KinectCVShapeSample.cpp
	$(CPP) -I/usr/include/oscpack/ KinectCVShapeSample.cpp -o KinectCVShapeSample $(SHAREDFLAGS) -loscpack

playScan: KinectCVShapeScan fromsample.pl manygrain.csd
	./KinectCVShapeScan | perl fromcv.pl | csound manygrain.csd


playSample: KinectCVShapeSample fromsample.pl manygrain.csd
	./KinectCVShapeSample | perl fromsample.pl | csound manygrain.csd

playSampleOSC: KinectCVShapeSample fromsample-osc.pl manygrain-4101-0.csd manygrain-4102-0.csd manygrain-4103-0.csd manygrain-4104-0.csd csound-starter.sh
	./KinectCVShapeSample | perl fromsample-osc.pl | bash csound-starter.sh manygrain-4101-0.csd manygrain-4102-0.csd manygrain-4103-0.csd manygrain-4104-0.csd


playMultiScan: KinectCVShapeMultiScan fromlines.pl manygrain.csd
	./KinectCVShapeMultiScan | perl fromlines.pl | csound manygrain.csd

playMultiScanOSC: KinectCVShapeMultiScan fromlines-osc.pl manygrain-4101-0.csd manygrain-4102-0.csd manygrain-4103-0.csd manygrain-4104-0.csd csound-starter.sh
	./KinectCVShapeMultiScan | perl fromlines-osc.pl | bash csound-starter.sh manygrain-4101-1.csd manygrain-4102-1.csd manygrain-4103-1.csd manygrain-4104-1.csd 



format:
	astyle --style=1tbs -s2 *.cpp *.h

manygrain-4101-0.csd manygrain-4102-0.csd manygrain-4103-0.csd manygrain-4104-0.csd manygrain-4101-1.csd manygrain-4102-1.csd manygrain-4103-1.csd manygrain-4104-1.csd: genmanygrains.pl manygrain.csd.tmpl
	perl genmanygrains.pl
