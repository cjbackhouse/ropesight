#!/usr/bin/python

import gtk
import os

def OnStart(but):
    pn = PlaceNotation()
    # TODO: doesn't work if gui is executed from wrong directory
    cmd = './rsight '+str(NumBells())+' '+PlaceNotation()+' '+str(PealTime())
#    print cmd
    os.system(cmd+' &')
    gtk.main_quit()

def NumBells():
    txt = stage.get_active_text()
    if txt == 'Doubles': return 5
    if txt == 'Minor':   return 6
    if txt == 'Triples': return 7
    if txt == 'Major':   return 8
    if txt == 'Royal':   return 10
    if txt == 'Maximus': return 12
    assert False # Unknown stage

def PlaceNotation():
    meth = method.get_active_text()
    num = NumBells()
    if meth == 'Plain Hunt':
        if num == 5:  return '5.1'
        if num == 6:  return 'x16'
        if num == 7:  return '7.1'
        if num == 8:  return 'x18'
        if num == 10: return 'x10'
        if num == 12: return 'x1T'
    if meth == 'Plain Bob':
        if num == 5:  return '5.1.5.1.5.1.5.1.5.125'
        if num == 6:  return 'x16x16x16x16x16x12'
        if num == 7:  return '7.1.7.1.7.1.7.1.7.1.7.1.7.127'
        if num == 8:  return 'x18x18x18x18x18x18x18x12'
        if num == 10: return 'x10x10x10x10x10x10x10x10x10x12'
        if num == 12: return 'x1Tx1Tx1Tx1Tx1Tx1Tx1Tx1Tx1Tx1Tx1Tx12'
    if meth == 'Grandsire':
        if num%2 == 0: num = num-1
        if num == 5:  return '3.1.5.1.5.1.5.1.5.1'
        if num == 7:  return '3.1.7.1.7.1.7.1.7.1.7.1.7.1'
        if num == 9:  return '3.1.9.1.9.1.9.1.9.1.9.1.9.1.9.1.9.1'
        if num == 11: return '3.1.E.1.E.1.E.1.E.1.E.1.E.1.E.1.E.1.E.1.E.1'
    assert False # Unknown method

def PealTime():
    return int(hours.get_value()*60+minutes.get_value())

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
stage.append_text('Royal')
stage.append_text('Maximus')
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
