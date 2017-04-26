import sys
import colorsys
import subprocess
import re
import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.cbook as cbk
import matplotlib.lines as lines
import matplotlib.patches as patches
import matplotlib.ticker as ticker
from matplotlib import rc
import math
import numpy
import scipy
import scipy.integrate


class ColourPallete:
    @staticmethod
    def toHex(r,g,b):
        return '#%02x%02x%02x' % (r,g,b)
    @staticmethod
    def toRGB(hexval):
        value = hexval.lstrip('#')
        lv = len(value)
        return tuple(int(value[i:i+lv/3], 16) for i in range(0, lv, lv/3))
    @staticmethod
    def HSV_to_RGB(hsviter):
        #in colorsys all input coords go from 0 to 1
        #keep 0 to 1 range for hsv, but use 0-255 for RGB as usual
        rgb = colorsys.hsv_to_rgb(hsviter[0],hsviter[1],hsviter[2])

        #print "HSV_to_RGB: ",hsviter, rgb
        #return a list
        out = [1,1,1]
        for i in range(3):
            out[i] = rgb[i]*255.0
        return out
    @staticmethod
    def RGB_to_HSV(rgbiter):
        #see comment above
        hsv = colorsys.rgb_to_hsv(rgbiter[0]/255.0, rgbiter[1]/255.0, rgbiter[2]/255.0)
        out = [1,1,1]
        for i in range(3):
            out[i] = hsv[i]
        return out


    @staticmethod
    def generatePallete(nhues,startcolour=(0,1,1) ):
        #startcolour should be a tuple (h,s,v)
        #vary the hue but not the saturation or value
        sep = 1.0/nhues 
        rgbpal = []
        for i in range(nhues):
            hsv = [startcolour[0]+sep*i, startcolour[1], startcolour[2] ]
            print "HSV is ",hsv
            rgb = ColourPallete.HSV_to_RGB(hsv)
            rgbpal.append(ColourPallete.toHex(rgb[0],rgb[1], rgb[2] ) )
        return rgbpal


    @staticmethod
    def pasteliseColour(colour, alpha):
        colorrgb = colour
        if(cbk.is_string_like(colour)==True ):
            colorrgb = ColourPallete.toRGB(colour)

        colorhsv = ColourPallete.RGB_to_HSV(colorrgb)
        newrgb = ColourPallete.HSV_to_RGB([colorhsv[0],colorhsv[1]*alpha,colorhsv[2]])
        return ColourPallete.toHex(newrgb[0],newrgb[1],newrgb[2])


    @staticmethod
    def fixedPallete(nhues):
        """Return a list of $nhues colours (as hex in rgb) to use as a pallete chosen to be distinct"""
        pallete = [
            '#ff0000',
            '#00ff00',
            '#0000ff',
            '#ff9c00',
            '#a3007f',
            '#00a000',
            '#ff00ff',
            '#00ffff',
            '#d38d4e']
        if(nhues > len(pallete)):
            raise IndexError, "Not enough predefined colours in pallete\n";
        return pallete[:nhues]

    @staticmethod
    def palleteMap(keylist, sort_cmp = None, reverse = False):
        """Return a dictionary mapping each key to a colour from the static list. If sort_cmp != None, a sort
        is first performed on the keylist using the sort_cmp function. cmp puts the smallest number at the top of the list!
        Use reverse=True to reverse the list post-sort"""

        lst = keylist
    
        if(sort_cmp !=None):
            lst.sort(sort_cmp)
            if(reverse==True):
                lst.reverse()

        out = {}
        pidx = 0
        for i in range(len(lst)):
            if(i>0):
                #check for multiple entries with same value, assign same colour
                if(sort_cmp != None and sort_cmp(lst[i],lst[i-1])==0):
                    print lst[i], lst[i-1]
                    out[lst[i]] = out[lst[i-1]]
                elif(sort_cmp == None and lst[i] == lst[i-1]):
                    out[lst[i]] = out[lst[i-1]]
                else:
                    out[lst[i]] = pidx
                    pidx+=1
            else:
                out[lst[i]] = pidx
                pidx+=1

        pallete = ColourPallete.fixedPallete(pidx)        
        for i in range(len(lst)):
            out[lst[i]] = pallete[out[lst[i] ] ]

        return out

class DataSet:
        def __init__(self):
                self.x = None
                self.y = None
                self.dxm = None
                self.dxp = None
                self.dym = None
                self.dyp = None

class ErrorBand:
        def __init__(self):
                self.x = None
                self.upper = None
                self.lower = None

def plotDataSet(axes,dataset, **kwargs):
    if 'linestyle' not in kwargs.keys():
        kwargs['linestyle'] = ""
    if 'marker' not in kwargs.keys():
        kwargs['marker'] = "o"
    if 'ms' not in kwargs.keys():
        kwargs["ms"] = 5 #marker size

    #My extra arguments
    capwidth = 2.0
    bar_linestyle = 'solid' #ACCEPTS: ['solid' | 'dashed', 'dashdot', 'dotted' | (offset, on-off-dash-seq) ]
    color = 'r'
    hollowsymbol = False
 
    if 'capwidth' in kwargs.keys():
        capwidth = kwargs['capwidth']
        del kwargs['capwidth']
    if 'bar_linestyle' in kwargs.keys():
        bar_linestyle = kwargs['bar_linestyle']
        del kwargs['bar_linestyle']
    if 'hollowsymbol' in kwargs.keys():
        hollowsymbol = kwargs['hollowsymbol']
        del kwargs['hollowsymbol']
    if 'color' in kwargs.keys():
        color = kwargs['color']
        del kwargs['color']

    if 'ecolor' not in kwargs.keys():
        kwargs['ecolor'] = color
    if 'mfc' not in kwargs.keys():
        kwargs['mfc'] = color

    if(len(dataset.x)==0):
        return

    x = dataset.x
    y = dataset.y
    dx = [dataset.dxm,dataset.dxp]
    dy = [dataset.dym,dataset.dyp]

    plotset = None
    try:
        plotset = axes.errorbar(x,y,xerr=dx,yerr=dy,**kwargs)
    except Exception, Message:
        raise  Exception, "Error plotting dataset %s" % (Message)

    #bar linestyle
    plotset[2][0].set_linestyles(bar_linestyle)
    plotset[2][1].set_linestyles(bar_linestyle)

    #capsize
    for cap in plotset[1]:
        plt.setp(cap,mew=capwidth)
    
    #Hollow symbols if required
    if(hollowsymbol == True):
        s = plotset[0]
        s.set_markerfacecolor("None")
        s.set_markeredgecolor(color)
        if 'markeredgewidth' not in kwargs.keys():
            s.set_markeredgewidth(1.25)

    return plotset



def plotErrorBand(axes, band, **kwargs):
    x = band.x
    uy = band.upper
    ly = band.lower

    usetransparency=False
    boundary_lines = False
    boundary_lines_zorder = 10

    if "usetransparency" in kwargs.keys():
        usetransparency = kwargs["usetransparency"]
        del kwargs["usetransparency"]
    if "boundary_lines" in kwargs.keys():
        boundary_lines = kwargs["boundary_lines"]
        del kwargs["boundary_lines"]
    if "boundary_lines_zorder" in kwargs.keys():
        boundary_lines_zorder = kwargs["boundary_lines_zorder"]
        del kwargs["boundary_lines_zorder"]
        
    plot_band = axes.fill_between(x,ly,uy,**kwargs)

    if(boundary_lines == True):
        colour = plt.getp(plot_band,'facecolors')
        chex = ColourPallete.toHex(colour[0][0]*255,colour[0][1]*255,colour[0][2]*255)
        axes.plot(x,uy,marker='None',linestyle='--',linewidth=1.5,color=chex,zorder=boundary_lines_zorder)
        axes.plot(x,ly,marker='None',linestyle='--',linewidth=1.5,color=chex,zorder=boundary_lines_zorder)

    return plot_band