#!/usr/bin/env python
import wx
import cStringIO
import os
import re

class Classifier(wx.Frame):
    def __init__(self, parent, title):
        super(Classifier, self).__init__(parent, title=title, style=wx.DEFAULT_FRAME_STYLE ^ wx.RESIZE_BORDER)
        
        # Do not allow a smaller window than that
        #self.SetMinSize((512,400));
        
        self.Bind(wx.EVT_KEY_DOWN, self.keyboard)
        
        # Initialise the global variables
        self.reset()
        
        self.initUI()
        
        self.Centre()
        self.Show()
    
    def reset(self):
        self.images = []
        self.imageIndex = 0
        self.sampleDirectory = ""
        self.dragging = False
        self.dragStart = [0, 0]
        self.dragStop = [0, 0]
        self.selectedObjects = []
        self.currentObject = False
        self.samples = []
    
    def initUI(self):
        #panel = wx.Panel(self)
        
        sizer = wx.GridBagSizer(1, 2)
        
        # Add the top toolbar with the control buttons
        toolbar = wx.ToolBar(self, -1, style=wx.TB_HORIZONTAL | wx.NO_BORDER)
        
        toolbar.AddSimpleTool(1, wx.Image('ui/stock_open.png', wx.BITMAP_TYPE_PNG).ConvertToBitmap(), 'Open the directory containing the sample images', '')
        self.Bind(wx.EVT_TOOL, self.toolbarOpen, id=1)
        toolbar.AddSimpleTool(2, wx.Image('ui/stock_save.png', wx.BITMAP_TYPE_PNG).ConvertToBitmap(), 'Save', '')
        self.Bind(wx.EVT_TOOL, self.toolbarSave, id=2)
        toolbar.AddSeparator()
        toolbar.AddSimpleTool(3, wx.Image('ui/stock_undo.png', wx.BITMAP_TYPE_PNG).ConvertToBitmap(), 'Undo last marking', '')
        self.Bind(wx.EVT_TOOL, self.toolbarUndo, id=3)
        toolbar.AddSimpleTool(4, wx.Image('ui/stock_clear.png', wx.BITMAP_TYPE_PNG).ConvertToBitmap(), 'Clear', '')
        self.Bind(wx.EVT_TOOL, self.toolbarClear, id=4)
        toolbar.AddSeparator()
        toolbar.AddSimpleTool(5, wx.Image('ui/stock_left_arrow.png', wx.BITMAP_TYPE_PNG).ConvertToBitmap(), 'Previous image', '')
        self.Bind(wx.EVT_TOOL, self.toolbarPrev, id=5)
        toolbar.AddSimpleTool(6, wx.Image('ui/stock_negative.png', wx.BITMAP_TYPE_PNG).ConvertToBitmap(), 'Set image as negative', '')
        self.Bind(wx.EVT_TOOL, self.toolbarNegative, id=6)
        toolbar.AddSimpleTool(7, wx.Image('ui/stock_right_arrow.png', wx.BITMAP_TYPE_PNG).ConvertToBitmap(), 'Next image', '')
        self.Bind(wx.EVT_TOOL, self.toolbarNext, id=7)
        toolbar.AddSeparator()
        toolbar.AddSimpleTool(8, wx.Image('ui/stock_preferences.png', wx.BITMAP_TYPE_PNG).ConvertToBitmap(), 'Preferences', '')
        toolbar.AddSimpleTool(9, wx.Image('ui/stock_info.png', wx.BITMAP_TYPE_PNG).ConvertToBitmap(), 'About', '')
        self.Bind(wx.EVT_TOOL, self.toolbarInfo, id=9)
        toolbar.AddSimpleTool(10, wx.Image('ui/stock_exit.png', wx.BITMAP_TYPE_PNG).ConvertToBitmap(), 'Exit', '')
        self.Bind(wx.EVT_TOOL, self.toolbarExit, id=10)
        toolbar.Realize()
        sizer.Add(toolbar, pos=(0,0), border=5, flag=wx.EXPAND)
        
        self.image = wx.StaticBitmap(self, bitmap=wx.EmptyBitmap(320, 240), pos=(0, 0))
        # Bind mouse actions on the image
        self.image.Bind(wx.EVT_MOUSE_EVENTS, self.imageMouseEvent)
        sizer.Add(self.image, pos=(1, 0), flag=wx.ALL, border=0)
        
        # Ask user to save when closing the application
        self.Bind(wx.EVT_CLOSE, self.onClose)
        
        # Overlay to draw on top of the picture
        self.overlay = wx.Overlay()
        
        # Only the image part in the user interface should be resizable
        sizer.AddGrowableCol(0)
        sizer.AddGrowableRow(1)
        
        self.SetSizerAndFit(sizer)
        self.Layout()
    
    def imageMouseEvent(self, event):
        # See if there's a loaded picture
        if not len(self.images):
            return
        
        # See if user clicked on the image
        if event.ButtonDown():
            self.dragging = True
            self.dragStart = event.GetPosition()
            self.dragStop = event.GetPosition()
        elif event.ButtonUp() and self.currentObject:
            self.dragging = False
            
            # Clear the overlay
            self.overlay.Reset()
            
            # Smaller features than 10 by 10 pixels are not accepted
            if self.currentObject.GetWidth() < 10 or self.currentObject.GetHeight() < 10:
                self.currentObject = False
                return
            
            # Draw permanent marker on the picture itself
            self.drawRect(self.currentObject)
            
            # Save the last action
            self.selectedObjects.append(self.currentObject)
            self.currentObject = False

        # The user wants to delimit a feature
        if self.dragging:
            self.dragStop = event.GetPosition()
            
            dc = wx.ClientDC(self.image)
            odc = wx.DCOverlay(self.overlay, dc)
            odc.Clear()
            
            dc.SetPen(wx.Pen("red", style=wx.SOLID))
            dc.SetBrush(wx.Brush("red", wx.TRANSPARENT))
            w = abs(self.dragStart[0] - self.dragStop[0])
            h = abs(self.dragStart[1] - self.dragStop[1])
            
            # Make it so that the rectangle could be dragged in any way and constrain the rectangle to a square
            if self.dragStart[0] < self.dragStop[0]:
                x = self.dragStart[0]
            else:
                x = self.dragStop[0]
                
            if self.dragStart[1] < self.dragStop[1]:
                y = self.dragStart[1]
            else:
                y = self.dragStop[1]
            
            h = w
            
            self.currentObject = wx.Rect(x, y, w, h)
            dc.DrawRectangleRect(self.currentObject)

    # Toolbar button callbacks
    def toolbarOpen(self, event):
        self.sampleFolderSelect()
    
    def toolbarSave(self, event):
        self.save()
    
    def save(self):
        positives = ""
        negatives = ""
        for i in range(0, len(self.samples)):
            obj = len(self.samples[i]['rects'])
            if obj:
                objList = []
                for r in self.samples[i]['rects']:
                    objList.append(str(r.x) + " " + str(r.y) + " " + str(r.GetWidth()) + " " + str(r.GetHeight()))
                positives += self.samples[i]['path'] + " " + str(obj) + " " + " ".join(objList) + os.linesep
            else:
                negatives += self.samples[i]['path'] + os.linesep
        
        file = open("positive.txt", "w")
        file.write(positives)
        file.close()
        
        file = open("negative.txt", "w")
        file.write(negatives)
        file.close()
    
    def toolbarUndo(self, event):
        if not len(self.images):
            self.sampleFolderSelect()
            return
        
        # If the user has selected something undo the selection
        if len(self.selectedObjects):
            self.selectedObjects.pop()
        else:
            # Otherwise remove the last selection form the image entry
            path = self.sampleDirectory + "/" + self.images[self.imageIndex]
            for i in range(0, len(self.samples)):
                if path == self.samples[i]['path'] and len(self.samples[i]['rects']):
                    if len(self.samples[i]['rects']) > 1:
                        # Remove the last selected object
                        self.samples[i]['rects'].pop()
                    else:
                        # If no more items are selected remove the image or it will be considered as negative
                        self.samples.pop(i)
                    break
        
        # Reload the current image
        self.loadImage(0, self.imageIndex)
    
    def toolbarClear(self, event):
        if not len(self.images):
            self.sampleFolderSelect()
            return
            
        # Search for the image in the list and reset the rects field
        path = self.sampleDirectory + "/" + self.images[self.imageIndex]
        for i in range(0, len(self.samples)):
            if path == self.samples[i]['path']:
                self.samples.pop(i)
                break
        
        # Clear the currently selected objects as they are saved with loadImage()
        self.selectedObjects = []
        # Reload the current image
        self.loadImage(0, self.imageIndex)
    
    def toolbarNegative(self, event):
        self.sampleNegative()
        
    def toolbarPrev(self, event):
        self.loadImage(0)
        
    def toolbarNext(self, event):
        self.loadImage(1)
    
    def toolbarInfo(self, event):
        dialog = wx.MessageDialog(self, 'Haar feature selection application\n\nKarl Kangur 2014', 'Info', wx.OK | wx.ICON_INFORMATION)
        dialog.ShowModal()
        dialog.Destroy()
    
    def toolbarExit(self, event):
        self.Close()
    
    def keyboard(self, event):
        keycode = event.GetKeyCode()
        # Left and right arrow keys allow to navigate the images
        if keycode == wx.WXK_LEFT:
            self.loadImage(0)
        elif keycode == wx.WXK_RIGHT:
            self.loadImage(1)
        event.Skip()
    
    def onClose(self, event):
        if len(self.samples):
            dialog = wx.MessageDialog(self, "Do you wish to save before quitting the application?", "Save?", wx.OK|wx.CANCEL|wx.ICON_QUESTION)
            if dialog.ShowModal() == wx.ID_OK:
                self.save()
            dialog.Destroy()
        
        event.Skip()
        
    def sampleNegative(self):
        # Check if the file is already in the negative list
        path = self.sampleDirectory + "/" + self.images[self.imageIndex]
        # This also solves the problem of already having this image as positive
        line = {"path": path, "rects": []}
        
        exists = False
        for i in range(0, len(self.samples)):
            if path == self.samples[i]['path']:
                exists = True
                self.samples[i] = line
                break
    
        if not exists:
            self.samples.append(line)
        
        # Load next image
        self.loadImage(1)
        
    def sampleFolderSelect(self):
        # Let the user select the directory with all the sample images
        dialogDirectorySelect = wx.DirDialog(self, "Select the directory containing the samples")
        if dialogDirectorySelect.ShowModal() == wx.ID_OK:
            # Reset all the currently loaded values to global variables
            self.reset()
            
            # Set the new directory from which to fetch the pictures
            self.sampleDirectory = dialogDirectorySelect.GetPath()
            
            # Automatically fetch all the pictures and load the first one
            self.getImages()
            
            if not len(self.images):
                emptyBitmap = wx.EmptyBitmap(320, 240)
                self.image.SetBitmap(emptyBitmap)
                self.Fit()
                return
        
            # See if there's a positive.vec file
            if os.path.isfile("positive.txt"):
                # Open file in reading and appending mode
                positive = open("positive.txt", "r")
                self.parseSamples(positive.readlines())
                positive.close()
            
            # The same for negative.txt file
            if os.path.isfile("negative.txt"):
                # Open file in reading and appending mode
                negative = open("negative.txt", "r")
                self.parseSamples(negative.readlines())
                negative.close()
            
            self.loadImage(1, 0)
        
        dialogDirectorySelect.Destroy()
    
    def parseSamples(self, samples):
        for sample in samples:
            objects = re.search(r"^(.+?)((\s\d+)+)?$", sample)
            if objects != None:
                path = objects.group(1).strip()
                rects = objects.group(2)
                if rects:
                    rects = rects.strip().split(" ")
                    # First element shows the number of rectangles
                    number = int(rects.pop(0))
                    list = []
                    for i in range(0, number):
                        index = i * 4
                        x = int(rects[index])
                        y = int(rects[index + 1])
                        w = int(rects[index + 2])
                        h = int(rects[index + 3])
                        list.append(wx.Rect(x, y, w, h))
                    self.samples.append({"path" : path, "rects": list})
                else:
                    self.samples.append({"path" : path, "rects": []})

    def loadImage(self, direction, index = -1):
        # Images are not actually loaded yet
        if not len(self.images):
            self.sampleFolderSelect()
            return
        
        # Save the current image data if there is something selected
        self.saveMarkers()
        
        # Increment counter for next image
        if index == -1:
            if direction and self.imageIndex < len(self.images) - 1:
                self.imageIndex += 1
            elif not direction and self.imageIndex > 0:
                self.imageIndex -= 1
        
            # Define the image path
            path = self.sampleDirectory + "/" + self.images[self.imageIndex]
        else:
            path = self.sampleDirectory + "/" + self.images[index]
        
        # Check if there's actually a file there
        if not os.path.isfile(path):
            return
        
        # Load and show the image in the application
        image = wx.Image(path, wx.BITMAP_TYPE_ANY).ConvertToBitmap()
        self.image.SetBitmap(image)
        self.Fit()
        
        # Force refresh of window here to show the newly loaded image
        self.Update()

        # Draw the current markers on the picture if they exist
        for i in range(0, len(self.samples)):
            if path == self.samples[i]['path']:
                # Iterate through the rects list
                for rect in self.samples[i]['rects']:
                    self.selectedObjects.append(rect)
                    self.drawRect(rect)
                
                if not len(self.samples[i]['rects']):
                    self.drawNegative()
                
                break
    
    def drawRect(self, rect):
        dc = wx.PaintDC(self.image)
        dc.SetPen(wx.Pen("blue", style=wx.SOLID))
        dc.SetBrush(wx.Brush("blue", wx.TRANSPARENT))
        dc.DrawRectangleRect(rect)
    
    def drawNegative(self):
        dc = wx.PaintDC(self.image)
        ww, wh = self.image.GetSize()
        text = " Image marked as negative "
        tw, th = dc.GetTextExtent(text)
        dc.SetBackgroundMode(wx.SOLID)
        dc.SetTextBackground(wx.RED)
        dc.SetTextForeground(wx.WHITE)
        dc.DrawText(text, (ww - tw) / 2, (wh - th) / 2)

    def saveMarkers(self):
        if len(self.selectedObjects):
            # Path to image
            path = self.sampleDirectory + "/" + self.images[self.imageIndex]
            line = {"path": path, "rects": self.selectedObjects}
            
            # Check if the item is already in the positive list
            exists = False
            for i in range(0, len(self.samples)):
                # If the image path is somewhere in the current line then replace the whole line with the new data
                if path == self.samples[i]['path']:
                    exists = True
                    # Replace the old values by the new one
                    self.samples[i] = line
                    break
            
            if not exists:
                self.samples.append(line)
            
            # Reset the selected objects array for the new image
            self.selectedObjects = []

    def getImages(self):
        allowedExtentions = [".jpg", ".png"]
        for file in os.listdir(self.sampleDirectory):
            ext = os.path.splitext(file)[1]
            if ext in allowedExtentions:
                self.images.append(file)

        if not len(self.images):
            dialog = wx.MessageDialog(self, "Did not find any image in the selected folder", "Error", wx.OK | wx.ICON_ERROR)
            dialog.ShowModal()
            dialog.Destroy()

app = wx.App(False)
frame = Classifier(None, 'Sample creation application')
app.MainLoop()
