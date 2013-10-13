#!/usr/bin/python

import gtk
import os

def OnStart(but):
    # TODO: doesn't work if gui is executed from wrong directory
    os.system('./rsight &')
    gtk.main_quit()

win = gtk.Window()
win.set_title('Ropesight')

start = gtk.Button('Start ringing!')
img = gtk.Image()
img.set_from_stock(gtk.STOCK_MEDIA_PLAY, 1)
start.set_image(img)

vbox = gtk.VBox()

hbox = gtk.HBox()

hbox.pack_start(gtk.Label('Method:'), False)

method = gtk.combo_box_new_text()
method.append_text('Plain Hunt')
method.append_text('Plain Bob')
method.append_text('Grandsire')
method.set_active(0)
hbox.add(method)

stage = gtk.combo_box_new_text()
stage.append_text('Doubles')
stage.append_text('Minor')
stage.append_text('Triples')
stage.append_text('Major')
stage.set_active(1)
hbox.add(stage)

vbox.add(hbox)

hbox2 = gtk.HBox()
hbox2.pack_start(gtk.Label('Peal speed:'), False)

hours = gtk.SpinButton()
hours.set_numeric(True)
hours.set_range(0, 5)
hours.set_value(2)
hours.set_increments(1, 1)
hbox2.add(hours)
hbox2.pack_start(gtk.Label('hr'), False)
minutes = gtk.SpinButton()
minutes.set_numeric(True)
minutes.set_range(0, 59)
minutes.set_value(40)
minutes.set_increments(1, 10)
hbox2.add(minutes)
hbox2.pack_start(gtk.Label('min'), False)

vbox.add(hbox2)

hbox3 = gtk.HBox()
hbox3.pack_end(start, False)
vbox.add(hbox3)

win.add(vbox)

win.connect('destroy', gtk.main_quit)

start.connect('clicked', OnStart)

win.show_all()
gtk.main()
