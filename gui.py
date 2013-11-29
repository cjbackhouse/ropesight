#!/usr/bin/python

import gtk
import os

def OnStart(but):
    pn = PlaceNotation()
    # Add a tenor behind if necessary
    nb = NumBells()
    if nb%2 == 1: nb += 1
    # TODO: doesn't work if gui is executed from wrong directory
    cmd = './rsight '+str(nb)+' '+PlaceNotation()+' '+str(PealTime())+' '+str(Bell())+' '+str(Auto())
    print cmd
    win.hide()
    os.system(cmd)
    gtk.main_quit()

def NumBells():
    txt = stage.get_active_text()
    if txt == 'Doubles': return 5
    if txt == 'Minor':   return 6
    if txt == 'Triples': return 7
    if txt == 'Major':   return 8
    if txt == 'Caters':  return 9
    if txt == 'Royal':   return 10
    if txt == 'Cinques': return 11
    if txt == 'Maximus': return 12
    assert False # Unknown stage

def PlaceNotation():
    meth = method.get_active_text()
    num = NumBells()
    if meth == 'Rounds':
        return '-'
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
    if meth == 'Little Bob':
        if num == 6:  return 'x16x14x16x12'
        if num == 8:  return 'x18x14x18x12'
        if num == 10: return 'x10x14x10x12'
        if num == 12: return 'x1Tx14x1Tx12'
    if meth == 'Stedman':
        if num%2 == 0: num = num-1
        if num == 5:  return '3.1.5.3.1.3.1.3.5.1.3.1'
        if num == 7:  return '3.1.7.3.1.3.1.3.7.1.3.1'
        if num == 9:  return '3.1.9.3.1.3.1.3.9.1.3.1'
        if num == 11: return '3.1.E.3.1.3.1.3.E.1.3.1'
    if meth == 'St Clement\'s':
        if num == 6: return 'x16x36x36x36x16x12'
    if meth == 'Kent':
        if num == 6: return '14x14.16x12x16x12x16'
        if num == 8: return '14x14.18x12x18x12x18x12x18x12x18'
    if meth == 'Cambridge':
        if num == 6: return 'x36x14x12x36x14x56x14x36x12x14x36x12'
        if num == 8: return 'x36x14x1258x36x14x58x16x78x16x58x14x36x1258x14x38x12'
    assert False # Unknown method

def PealTime():
    return int(hours.get_value()*60+minutes.get_value())

def Bell():
    return int(bell.get_value())

def Auto():
    return int(auto.get_active())

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
method.append_text('Rounds')
method.append_text('Plain Hunt')
method.append_text('Plain Bob')
method.append_text('Grandsire')
method.append_text('Little Bob')
method.append_text('Stedman')
method.append_text('St Clement\'s')
method.append_text('Kent')
method.append_text('Cambridge')
method.set_active(1)
hbox.add(method)

stage = gtk.combo_box_new_text()
stage.append_text('Doubles')
stage.append_text('Minor')
stage.append_text('Triples')
stage.append_text('Major')
stage.append_text('Caters')
stage.append_text('Royal')
stage.append_text('Cinques')
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

hbox4 = gtk.HBox()
hbox4.pack_start(gtk.Label('Your bell:'), False)
bell = gtk.SpinButton()
bell.set_numeric(True)
bell.set_range(1, 12)
bell.set_value(2)
bell.set_increments(1, 1)
hbox4.add(bell)

auto = gtk.CheckButton('AI')
hbox4.add(auto)

vbox.add(hbox4)

hbox3 = gtk.HBox()
hbox3.pack_end(start, False)
vbox.add(hbox3)

win.add(vbox)

win.connect('destroy', gtk.main_quit)

start.connect('clicked', OnStart)

win.show_all()
gtk.main()
