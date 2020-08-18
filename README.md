# Image-Mosaics
Equal-size / Unequal-size block image mosaics

*****************************
*       Note        
*****************************

Two source files, one is for equal-size block image mosaics, the other one is for unequal-size block image mosaics.

The original src files are parts of a "windows form app" and some parts of code are removed.

Thus, the src files prob cannot been compiled successfully, codes are just for showing the steps.

A report is for all details and example output images/

*****************************
*       Author      
*****************************

Author: Qingshuai Feng

Supervisor: Jim Miles

Second Marker: Carole Twining


*****************************
*       Introduction        
*****************************

This image mosaic application is to use plenty of library images to combine a master image and the result image shows a mosaic effect. 

The app has two functions, one is to realise equal-size block imagemosaics which can apply original image overlay. The other one is to realise unequal-size block imagemosaics where we use smaller blocks for object and larger ones for background.

The steps and key points of the process and algorithm are shown in a pdf file. This application is created with C++, OpenCV under Visual Studio.


*****************************
*      Equal-size block     
*****************************

The souce code is mainly in MainForm.h.

*CAUTION*: To run this application, you need to have some dll file on your computer such as opencv_world330d.dll. Or it won't work.

There is a GUI for equal-size block one. library images are placed in a folder called library. Master images are placed in a folder called master. The result images will be written to a folder called target (EXAMPLES). Data of library images and master image will be stored in eleven yaml files.

To gerenate a imagemosaics: 
  Select a master image 
  Enter the target image name and number of library images 
  Choose the block size, overlay percentage and the resolution. 
  Click button Generate. 

The result image will appear at the right of the window and be written to folder called target when it is generated.


*****************************
*     Unequal-size block    
*****************************

The source code is mainly in imagemosaicunequal.cpp.

This is a terminal application for this part. library images are placed in a folder called library. Master images are placed in a folder called master. The result images will be written to a folder called target (EXAMPLES). Tempory images are written to a folder called tmp. Data of library images and master image will be stored in eleven yaml files.

To gerenate a imagemosaics:
  Enter the number of library images and master image name. 
  Then a window will appear. 
    Use mouse left-click to define a rectangle, make sure the object is inside it.
    Hold shift and use mouse right click to draw some lines on object area
    Hold ctrl and use mouse right click to draw some lines on background area
    Press 's' to start extract object
  Enter the target image name and the block number on row and colomn

The result image will be written to folder called target.
