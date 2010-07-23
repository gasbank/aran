#!/usr/bin/env python

import gtk, gtk.glade, gobject
import os, sys, time, traceback
import threading, Queue
import subprocess

class PymControlPanel:

    # This is a callback function. The data arguments are ignored
    # in this example. More on callbacks below.
    def hello(self, widget, data=None):
        print "Hello World"

    def delete_event(self, widget, event, data=None):
        # If you return FALSE in the "delete_event" signal handler,
        # GTK will emit the "destroy" signal. Returning TRUE means
        # you don't want the window to be destroyed.
        # This is useful for popping up 'are you sure you want to quit?'
        # type dialogs.
        print "delete event occurred"

        # Change FALSE to TRUE and the main window will not be destroyed
        # with a "delete_event".
        return False

    def destroy(self, widget, data=None):
        print "destroy signal occurred"
        gtk.main_quit()
        
    def __init__(self):
        """
        UI for the mp3val MP3 validator
        """
        # Load Glade
        gladefile = "PymControlPanel.glade"
        if not gladefile:
            gobject.idle_add(doErrormessage, "UI description file PymControlPanel.glade not found.")
            
        self.windowname = "PymControlPanelWindow"
        self.wTree = gtk.glade.XML(gladefile, self.windowname)
        
        
        # Signals map
        dic = {
            "mainwindowDestroy" : self.destroy,
            "on_button1_clicked" : self.destroy,
            #"buttonAddClicked" : self.addButton,
            #"onFixingToggled" : self.toggleFixing,
        }

        # connect signals
        self.wTree.signal_autoconnect(dic)

        self.window = self.wTree.get_widget(self.windowname)
        #self._messages = self.wTree.get_widget("textviewMessages").get_buffer()
        # prepare colors for list view
        #self._marker = []
        #self._marker.append(self._messages.create_tag('warning', background='yellow'))
        #self._marker.append(self._messages.create_tag('error', background='red'))

        #self.wTree.get_widget("toggleFixing").set_sensitive(True)

        #self.finder = finderThread(logger=self.log)
        #self.validator = validatorThread(logger=self.log)
        # link the threads
        #self.finder.addaction = self.validator.queue
        #self.validator.doneaction = self.updateProgress
        #self.validator.callbackCanEnableWrite = self.updateCanWrite


        # When the window is given the "delete_event" signal (this is given
        # by the window manager, usually by the "close" option, or on the
        # titlebar), we ask it to call the delete_event () function
        # as defined above. The data passed to the callback
        # function is NULL and is ignored in the callback function.
        self.window.connect("delete_event", self.delete_event)
    
        # Here we connect the "destroy" event to a signal handler.  
        # This event occurs when we call gtk_widget_destroy() on the window,
        # or if we return FALSE in the "delete_event" callback.
        self.window.connect("destroy", self.destroy)
        self._done = 0
        

    def __init__2(self):
        # create a new window
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.set_title("Pymuscle Control Panel")

    
        # When the window is given the "delete_event" signal (this is given
        # by the window manager, usually by the "close" option, or on the
        # titlebar), we ask it to call the delete_event () function
        # as defined above. The data passed to the callback
        # function is NULL and is ignored in the callback function.
        self.window.connect("delete_event", self.delete_event)
    
        # Here we connect the "destroy" event to a signal handler.  
        # This event occurs when we call gtk_widget_destroy() on the window,
        # or if we return FALSE in the "delete_event" callback.
        self.window.connect("destroy", self.destroy)
    
        # Sets the border width of the window.
        self.window.set_border_width(2)
    
        # Creates a new button with the label "Hello World".
        self.button = gtk.Button("Simulate")
        
        self.pbar = gtk.ProgressBar()
        self.pbar.set_fraction(0.4)
    
        # When the button receives the "clicked" signal, it will call the
        # function hello() passing it None as its argument.  The hello()
        # function is defined above.
        self.button.connect("clicked", self.hello, None)
    
        # This will cause the window to be destroyed by calling
        # gtk_widget_destroy(window) when "clicked".  Again, the destroy
        # signal could come from here, or the window manager.
        
        #self.button.connect_object("clicked", gtk.Widget.destroy, self.window)
    
        vbox = gtk.HBox(False, 0)
        vbox.set_border_width(10)
        self.window.add(vbox)
        vbox.show()
        
        # Create a centering alignment object
        align = gtk.Alignment(0.5, 0.5, 0, 0)
        vbox.pack_start(align, False, False, 5)
        align.show()
        align.add(self.pbar)
        self.pbar.show()
        
        vbox.pack_start(self.button, False, False, 0)
        self.button.show()
        #vbox.pack_start(self.pbar, False, False, 0)
        #self.pbar.show()
        
        # The final step is to display this newly created widget.
        
        
    
        # and the window
        self.window.show()
    def start(self):
        self.window.show()
        #self.finder.start()
        #self.validator.start()
        
# If the program is run directly or passed as an argument to the python
# interpreter then create a HelloWorld instance and show it
if __name__ == "__main__":
    pcp = PymControlPanel()
    pcp.start()
    # All PyGTK applications must have a gtk.main(). Control ends here
    # and waits for an event to occur (like a key press or mouse event).
    gtk.main()
