from PIL import Image
im = Image.open("maple_mini_radial.bmp")


pix_data = []
for i in range(im.size[0]):
    for j in range(im.size[1]):
        val = 255 - im.getpixel((i,j))[0]
        im.putpixel((i,j),(val,val,val))

        if (val != 0):
            pix_data.append([i,j,val])
print "Total Dataset = %i pixels, %i bytes" % (len(pix_data),3*len(pix_data))

histogram = [[x,[]] for x in range(256)]
for pixel in pix_data:
    for bin in histogram:
        if pixel[2] == bin[0]:
            bin[1].append([pixel[0],pixel[1]])
            continue
print "Len of histogram: %i" % (len(histogram))
histogram = filter(lambda x: len(x[1]) != 0,histogram)
print "Len of filtered histogram (non zero hisogram bins): %i" % (len(histogram))

total_pmass = sum([x[0]*len(x[1]) for x in histogram])
print "Total probability mass: %i" % (total_pmass)

total = 0
for bin in histogram:
    total += bin[0]*len(bin[1])
    bin.append(total)


points = histogram[0][1]
print "num_points: %i" % (len(points))
dstr = ""
for i,point in enumerate(points):
    dstr += "{%i,%i}," % (point[0],point[1])
    if (i%10) == 0:
        dstr += "\n"
print dstr

import sys
import pygame
from time import clock
import random

def bin_search(lst,val,f):
    """
    return the value of lst whose element x has f(x) is less than or equal to
    val but is also closest to it. f specifies which element of x to search over
    """
    lenlst = len(lst)
    if lenlst == 1:
        return lst[0]

    if f(lst[lenlst/2-1]) >= val:
#        print "f(x): ",f(lst[lenlst/2 -1])
#        print "lst[%i]=%i greater than %i" % (lenlst/2,f(lst[lenlst/2]),val)
        return bin_search(lst[0:lenlst/2],val,f)
    else:
#        print "lst[%i]=%i less than %i" % (lenlst/2,f(lst[lenlst/2]),val)
        return bin_search(lst[lenlst/2:],val,f)

def get_random_point():
    rcoin = random.randint(0,total_pmass)
#    print "rcoin: ",rcoin
    r_points = bin_search(histogram,rcoin,lambda x: x[2])
#    print "rcoin: %i, cmass: %i, points: %s" % (rcoin,r_points[2],str(r_points[1]))
#    print "r_points: ", r_points
    ri = random.randint(0,len(r_points[1])-1)
    return r_points[1][ri][0],r_points[1][ri][1]

def get_random_point2():
    pix = pix_data[random.randint(0,len(pix_data)-1)]
    return pix[0],pix[1]

random.seed()

class Capacitor():
    def __init__(self,r,c,v0,max=255):
        self.r = r*1
        self.c = c*1.0
        self.x = v0*1.0
        self.max = max

    def update_dt(self,u,dt):
        xdot = (u-self.x)/(self.r*self.c*1.0)*1.0
        self.x += xdot*dt
        self.x = min(self.x,self.max)
        self.x = max(0,self.x)
        return self.x

class Pixel():
    def __init__(self, rect, r,c,v0):
        self.cap = Capacitor(r,c,0)
        self.rect = rect

    def set_r(self, r):
        self.cap.r = r

    def draw(self,u,dt,draw_now):
        if (draw_now):
            lum = min(255,self.cap.update_dt(u,dt))
            pygame.draw.rect(window,(lum,lum,lum),(self.rect))
        else:
            self.update_dt(u,dt)

    def update_dt(self,u,dt):
        self.cap.update_dt(u,dt)

def getNumber(key):
    numbers = [pygame.K_0,pygame.K_1,pygame.K_2,pygame.K_3,pygame.K_4,pygame.K_5,pygame.K_6,pygame.K_7,pygame.K_8,pygame.K_9]
    kpNumbers = [pygame.K_KP0,pygame.K_KP1,pygame.K_KP2,pygame.K_KP3,pygame.K_KP4,pygame.K_KP5,pygame.K_KP6,pygame.K_KP7,pygame.K_KP8,pygame.K_KP9]
    if key in numbers:
        return numbers.index(key)
    elif key in kpNumbers:
        return kpNumbers.index(key)
    else:
        return -1

def gen_pix_grid(rows,cols):
    dx = WIDTH/cols
    dy = HEIGHT/rows
    pixels = []

    for i in xrange(cols):
        for j in xrange(rows):
            xp = i*dx
            yp = j*dy
            rect = (xp,yp,dx,dy)
            r = 27
            c = random.randint(1,100)*.05/100 + 1
            this_pix = Pixel(rect,r,c,0)
            pixels.append(this_pix)
    return pixels

def in_rect(rect,pos):
    if pos[0] < rect[0]:
        return False
    if pos[0] > rect[0] + rect[2]:
        return False
    if pos[1] < rect[1]:
        return False
    if pos[1] > rect[1] + rect[3]:
        return False

    return True

def find_clicked_pixel(pixels,x,y):
    for pixel in pixels:
        if in_rect(pixel.rect,(x,y)):
            return pixel


#if __name__=="__main__":
if 1:
    WIDTH = 640
    HEIGHT = 640
    ROWS = 64
    COLS = 64
    SIGMA = 28
    LASER_FORCE = 120000
    REFRESH_RATE = 1/15.0
    DT = .05
    RPTS_PER_DT = 3

    pygame.init()
    window = pygame.display.set_mode((WIDTH,HEIGHT))


    u = 0
    now = clock()
    refresh = now
    draw_now = False

    random.seed()
    pixels = gen_pix_grid(ROWS,COLS)
    live_pixels = []
    while True:
        later = clock()
        dt = later-now
        now = later

        if (later - refresh) > REFRESH_RATE:
            draw_now = True
            refresh = later

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit(0)
            elif event.type == pygame.KEYDOWN:
                number = getNumber(event.key)
                if (number != -1):
                    for pix in pixels:
                        pix.set_r(number*3)
                elif event.key == pygame.K_s:
                    if (u > 0):
                        u = 0
                    else:
                        u = LASER_FORCE

        for i in xrange(RPTS_PER_DT):
            rx, ry = get_random_point()
            live_pixels.append(find_clicked_pixel(pixels,rx*10,ry*10))

        for pix in pixels:
            if live_pixels.count(pix) != 0:
                pix.draw(LASER_FORCE,dt,draw_now)
            else:
                pix.draw(u,dt,draw_now)

        live_pixels = []
        if (draw_now):
            pygame.display.flip()
            draw_now = False
