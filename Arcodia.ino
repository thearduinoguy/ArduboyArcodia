// Arcodia by Mike McRoberts aka TheArduinoGuy
// Port of the classic ZX Spectrum game 'Arcadia'.

#define SAVELOCATION (EEPROM_STORAGE_SPACE_START + 588)
#define maxNumMonsters 12

#include <Arduboy2.h>
#include "bitmaps.h"
//#include <avr/pgmspace.h>
#include <EEPROM.h>
#include "Sprites.h"

Arduboy2 arduboy;

Sprites sprites;

char shipX=58;
char shipY=47;
char numMonsters=6;
byte levelTimer=60;
int level=1;
byte monstersIndex=1;
byte cycle = 0;
// 1=Arrows,  2=Butterflys,  3=UFO,  4=Whirlygig,  5=Birds,  6=Spinnybox, 7=Radar, 8=Rocker, 9=Squiddy, 10=Bubble, 11=Wing, 12=Polo

long tick;
char lives=4;
long score, highScore;

struct laserBeams {
        char x;
        char y;
        unsigned char age;
};

laserBeams lasers[8];
byte laserIndex=0;
boolean laserFlip = 0;

struct movingObjects {
        float x;
        float y;
        byte state;
        boolean directionX;
        boolean directionY;
        char horizontalBarrier;
        long timer;
};

movingObjects monsters[20];

long lastPressed;
long monsterAnimRate;

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void setup()
{
        //Serial.begin(115200);
        arduboy.begin();
        arduboy.audio.on();
        arduboy.initRandomSeed();
        arduboy.setFrameRate(60);

        lastPressed = millis();
        monsterAnimRate = millis();
        tick = millis();
        menu();
        EEPROM.get(SAVELOCATION, highScore);
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void drawShip()
{
        sprites.drawSelfMasked( shipX, shipY, SHIP, 0);
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void initialiseLasers()
{
        for (int index=0; index<8; index++)
        {
                lasers[index].age=0;
        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void initialiseMonsters()
{
        for (int index=0; index<20; index++)
        {
                monsters[index].state = random(1,2);

                switch(monstersIndex)
                {
                case 1: // arrows
                        monsters[index].x = random(32, 159);
                        monsters[index].y = random(-64, -8);
                        break;
                case 2: // butterfly
                case 7: // radar

                        monsters[index].x =random(117);
                        monsters[index].y = random(-64, -16);
                        monsters[index].directionX=random(2);
                        break;
                case 3:  // UFO
                case 8: // Rockers
                        monsters[index].x =random(116);
                        monsters[index].y = -18;
                        monsters[index].directionX=random(2);
                        break;
                case 4: // Whirlygig
                        monsters[index].x =random(122);
                        monsters[index].y = random( -64, -16);
                        monsters[index].state = random(1, 4);
                        break;
                case 5:         // Birds
                        monsters[index].x =random(117);
                        monsters[index].y = random( -64, -16);
                        monsters[index].state = random(1, 5);
                        break;
                case 6:                 // Spinnybox
                        if (index<=14)
                        {
                                monsters[index].y = -10;
                                monsters[index].directionX=1;
                                monsters[index].x =(index*8);
                        }
                        else
                        {
                                monsters[index].y = -22;
                                monsters[index].directionX=0;
                                monsters[index].x =127-(index*8);
                        }
                        monsters[index].state = 1;
                        break;
                case 9:
                        monsters[index].x =random(117);
                        monsters[index].y = -10;
                        monsters[index].directionX=random(2);
                        monsters[index].directionY=1;
                        break;
                case 10:
                case 12:
                        monsters[index].x =random(117);
                        monsters[index].y = -10;
                        monsters[index].directionX=0;
                        monsters[index].directionY=1;
                        break;
                case 11:         // Wings
                        monsters[index].x =random(117);
                        monsters[index].y = -5;
                        monsters[index].directionX=random(2);
                        monsters[index].timer=3600000;
                        monsters[index].horizontalBarrier=random(32,53);
                        monsters[index].state=10;
                        break;
                }
        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void checkButtons()
{

        if (arduboy.pressed(LEFT_BUTTON)==true) shipX=shipX-2;
        if (arduboy.pressed(RIGHT_BUTTON)==true) shipX=shipX+2;
        if (arduboy.pressed(A_BUTTON)==true)
        {
                shipY=shipY-2;
                byte flameFrame=random(3);
                sprites.drawSelfMasked( shipX+2, shipY+19, FLAME, flameFrame);
                digitalWrite(2, HIGH); //positive square wave
                digitalWrite(5, LOW);              //positive square wavedelayMicroseconds(start);      //192uS
                delayMicroseconds(random(50));
                digitalWrite(2, LOW);              //neutral square wave
                digitalWrite(5, HIGH);              //positive square wave
                delayMicroseconds(random(50)+100);              //192uS}
        }
        if (arduboy.notPressed(A_BUTTON)==true) shipY=shipY+1;

        if (shipX<0) shipX=0;
        if (shipX>111) shipX = 111;
        if (shipY<4) shipY=4;
        if (shipY==4) {
                if (random(5)==3) shipY=shipY+((random(2)+1));
        }
        if (shipY>47) shipY=47;

        if (arduboy.pressed(B_BUTTON)==true) firelaser();
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void firelaser()
{
        if ((millis()-lastPressed)<100) return;
        lasers[laserIndex].age = 1;
        lasers[laserIndex].x = (laserFlip ? shipX : shipX+10);
        laserFlip = !laserFlip;
        lasers[laserIndex].y = shipY+4;
        laserIndex++;
        if (laserIndex==8) laserIndex=0;
        lastPressed=millis();
        for (int start = 25; start < 75; start = start + 1) {
                digitalWrite(2, HIGH); //positive square wave
                digitalWrite(5, LOW); //positive square wavedelayMicroseconds(start);      //192uS
                delayMicroseconds(start);
                digitalWrite(2, LOW); //neutral square wave
                digitalWrite(5, HIGH); //positive square wave
                delayMicroseconds(start); //192uS
        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void drawlasers()
{
        for (int index=0; index<8; index++)
        {
                if (lasers[index].age > 0 && lasers[index].age <8) sprites.drawSelfMasked( lasers[index].x, lasers[index].y, LASER1, 0);
                if (lasers[index].age >=8 && lasers[index].age <16) sprites.drawSelfMasked( lasers[index].x, lasers[index].y+3, LASER2,  0);
                if (lasers[index].age ==16) sprites.drawSelfMasked( lasers[index].x-4, lasers[index].y, POP, 0);
                if (lasers[index].age ==17) sprites.drawSelfMasked( lasers[index].x-4, lasers[index].y, POP, 1);
        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void drawMonsters()
{
        for (int index=0; index<numMonsters; index++)
        {
                if (monsters[index].state>0)
                {
                        byte animFrame = (monsters[index].state)-1;
                        switch(monstersIndex)
                        {

                        case 1:
                                sprites.drawSelfMasked( monsters[index].x, monsters[index].y,ARROW, animFrame);
                                break;
                        case 2:
                                sprites.drawSelfMasked( monsters[index].x, monsters[index].y,BUTTERFLY, animFrame);
                                break;
                        case 3:
                                sprites.drawSelfMasked( monsters[index].x, monsters[index].y, UFO, animFrame);
                                break;
                        case 4:
                                sprites.drawSelfMasked( monsters[index].x, monsters[index].y, WHIRLYGIG, animFrame);
                                break;
                        case 5:
                                sprites.drawSelfMasked( monsters[index].x, monsters[index].y, BIRDY, animFrame);
                                break;
                        case 6:
                                sprites.drawSelfMasked( monsters[index].x, monsters[index].y, SPINNYBOX, animFrame);
                                break;
                        case 7:
                                sprites.drawSelfMasked( monsters[index].x, monsters[index].y, RADAR, animFrame);
                                break;
                        case 8:
                                sprites.drawSelfMasked( monsters[index].x, monsters[index].y, ROCKER,  animFrame);
                                break;
                        case 9:
                                sprites.drawSelfMasked( monsters[index].x, monsters[index].y, SQUIDDY, animFrame);
                                break;
                        case 10:
                                sprites.drawSelfMasked( monsters[index].x, monsters[index].y, BUBBLE, animFrame);
                                break;
                        case 11:
                                sprites.drawSelfMasked( monsters[index].x, monsters[index].y, WING, 0);
                                break;
                        case 12:
                                sprites.drawSelfMasked( monsters[index].x, monsters[index].y, POLO, animFrame);
                                break;

                        }
                }

        }

}

void shipLand()
{
        shipX=58;
        for (int y=-10; y<47; y++)
        {
                arduboy.clear();
                sprites.drawSelfMasked( shipX, y, SHIP, 0);
                byte flameFrame=random(3);
                sprites.drawSelfMasked( shipX+2, y+18, FLAME, flameFrame);
                //Serial.write(arduboy.getBuffer(), 128 * 64 / 8);
                arduboy.display();
                for (int buzz=0; buzz<150; buzz++)
                {            digitalWrite(2, HIGH); //positive square wave
                             digitalWrite(5, LOW); //positive square wavedelayMicroseconds(start);      //192uS
                             delayMicroseconds(((y+10)*2)+random(100));
                             digitalWrite(2, LOW); //neutral square wave
                             digitalWrite(5, HIGH); //positive square wave
                             delayMicroseconds(((y+10)*2)+random(100)); //192uS}
                }
        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void moveThings()
{
        // Move the laser beams
        for (int index=0; index<8; index++)
        {
                if (lasers[index].age > 0)
                {
                        lasers[index].age++;
                        if (random(5)==3) lasers[index].age++;
                        if (lasers[index].age <16) lasers[index].y=lasers[index].y-3;
                        if (lasers[index].age >17) lasers[index].age=0;
                }
        }

        // move the monsters
        for (int index=0; index<numMonsters; index++)
        {
                if (monsters[index].state > 0)
                {
                        switch(monstersIndex)
                        {
                        case 1: // arrows - Move left and down
                                monsters[index].x = monsters[index].x -(random(2)+(cycle*0.1));
                                if (random(2)) monsters[index].y = monsters[index].y + (random(2)+(cycle*0.1));
                                if (monsters[index].x < -10)
                                {
                                        monsters[index].x = 128;
                                        //monsters[index].y=random(32)-64;
                                }
                                if (monsters[index].y >63)
                                {
                                        monsters[index].y = -8;
                                }
                                break;
                        case 2: // butterflys - move down and random change direction (left/right only)
                        case 7: // 7=Radar,
                                monsters[index].x = monsters[index].x + (monsters[index].directionX ? 0.75+(cycle*0.1) : -0.75+(cycle*0.1));
                                if (random(20)==10) monsters[index].directionX = random(2);
                                if (monsters[index].x <= 0)
                                {
                                        monsters[index].x = 0;
                                        monsters[index].directionX = !monsters[index].directionX;
                                }
                                if (monsters[index].x >= 117)
                                {
                                        monsters[index].x = 117;
                                        monsters[index].directionX = !monsters[index].directionX;
                                }
                                if (monsters[index].y >63)
                                {
                                        monsters[index].y = -8;
                                }
                                monsters[index].y += 0.3;
                                break;
                        case 3:         // UFOs - move down and random change direction (all directions)
                                monsters[index].x = monsters[index].x + (monsters[index].directionX ? .75+(cycle*0.1) : -.75+(cycle*0.1));
                                if (random(10)==5) monsters[index].directionX = random(2);
                                monsters[index].y = monsters[index].y + (monsters[index].directionY ? 0.3+(cycle*0.1) : -0.15+(cycle*0.1));
                                if (random(10)==5) monsters[index].directionY = random(2);

                                if (monsters[index].x <= 0)
                                {
                                        monsters[index].x = 0;
                                        monsters[index].directionX = !monsters[index].directionX;
                                }
                                if (monsters[index].x >= 116)
                                {
                                        monsters[index].x = 116;
                                        monsters[index].directionX = !monsters[index].directionX;
                                }
                                if (monsters[index].y >63)
                                {
                                        monsters[index].y = -10;
                                }
                                if (monsters[index].y < -12)
                                {
                                        monsters[index].y = -10;
                                }
                                break;
                        case 4:                 // WHIRLYGIGs - move down only

                                monsters[index].y = monsters[index].y + 0.3+(cycle*0.1);


                                if (monsters[index].y >63)
                                {
                                        monsters[index].y = -8;
                                }
                                if (monsters[index].state==2)
                                {
                                        monsters[index].x+=0.1+(cycle*0.1);
                                        if (random(2)) monsters[index].x+=0.1+(cycle*0.1);
                                }
                                if (monsters[index].state==4)
                                {
                                        monsters[index].x-=0.1+(cycle*0.1);
                                        if (random(2)) monsters[index].x-=0.1+(cycle*0.1);
                                }
                                break;
                        case 5:         // Birdys
                                monsters[index].x = monsters[index].x + (monsters[index].directionX ? 0.1+(cycle*0.1) : -0.1+(cycle*0.1));
                                if (random(50)==25) monsters[index].directionX = random(2);
                                if (monsters[index].x <= 0)
                                {
                                        monsters[index].x = 0;
                                        monsters[index].directionX = !monsters[index].directionX;
                                }
                                if (monsters[index].x >= 117)
                                {
                                        monsters[index].x = 117;
                                        monsters[index].directionX = !monsters[index].directionX;
                                }
                                if (monsters[index].y >63)
                                {
                                        monsters[index].y = -8;
                                }
                                monsters[index].y += 0.2+(cycle*0.1);
                                break;
                        case 6: // 6=Spinnybox,
                                monsters[index].x = monsters[index].x + (monsters[index].directionX ? 0.75+(cycle*0.1) : -0.75+(cycle*0.1));
                                if (monsters[index].directionX==1 && monsters[index].x>116)
                                {
                                        monsters[index].x=116;
                                        monsters[index].directionX=0;
                                        monsters[index].y+=8;
                                }
                                if (monsters[index].directionX==0 && monsters[index].x<1)
                                {
                                        monsters[index].x=1;
                                        monsters[index].directionX=1;
                                        monsters[index].y+=8;
                                }
                                break;
                        case 8: // 8=Rocker,
                                monsters[index].x = monsters[index].x + (monsters[index].directionX ? 0.25+(cycle*0.1) : -0.25+(cycle*0.1));
                                monsters[index].y+=0.25+(cycle*0.1);
                                if (monsters[index].y>63) monsters[index].y = -8;

                                if (monsters[index].directionX==1 && monsters[index].x>127) monsters[index].x = -11;
                                if (monsters[index].directionX==0 && monsters[index].x<-11) monsters[index].x=128;
                                break;
                        case 9:                 // Squiddys - move down and random change direction (all directions)
                                monsters[index].x = monsters[index].x + (monsters[index].directionX ? 0.75+(cycle*0.1) : -0.75+(cycle*0.1));
                                if (random(50)==25) monsters[index].directionX = random(2);
                                monsters[index].y = monsters[index].y + (monsters[index].directionY ? 0.3+(cycle*0.1) : -0.15+(cycle*0.1));
                                if (random(50)==25) monsters[index].directionY = random(2);

                                if (monsters[index].x <= 0)
                                {
                                        monsters[index].x = 0;
                                        monsters[index].directionX = !monsters[index].directionX;
                                }
                                if (monsters[index].x >= 117)
                                {
                                        monsters[index].x = 117;
                                        monsters[index].directionX = !monsters[index].directionX;
                                }
                                if (monsters[index].y >63)
                                {
                                        monsters[index].y = -8;
                                }
                                if (monsters[index].y < -12)
                                {
                                        monsters[index].y = -12;
                                        monsters[index].directionY=1;
                                }
                                break;
                        case 10:                         // bubbles - Move up down left right in sequence

                                if (monsters[index].directionX==0 && monsters[index].directionY==1)
                                {
                                        monsters[index].y+=0.35+(cycle*0.1);
                                        if(random(25)==12)
                                        {
                                                monsters[index].directionX=1;
                                                monsters[index].directionY=0;
                                                break;
                                        }
                                }
                                if (monsters[index].directionX==1 && monsters[index].directionY==0)
                                {
                                        monsters[index].x+=0.25+(cycle*0.1);
                                        if(random(25)==12)
                                        {
                                                monsters[index].directionX=0;
                                                monsters[index].directionY=0;
                                                break;
                                        }
                                }
                                if (monsters[index].directionX==0 && monsters[index].directionY==0)
                                {
                                        monsters[index].y-=0.25+(cycle*0.1);
                                        if(random(25)==12)
                                        {
                                                monsters[index].directionX=1;
                                                monsters[index].directionY=1;
                                                break;
                                        }
                                }
                                if (monsters[index].directionX==1 && monsters[index].directionY==1)
                                {
                                        monsters[index].x-=0.25+(cycle*0.1);
                                        if(random(25)==12)
                                        {
                                                monsters[index].directionX=0;
                                                monsters[index].directionY=1;
                                                break;
                                        }
                                }
                                if (monsters[index].x<-10) monsters[index].x=128;
                                if (monsters[index].x>127) monsters[index].x=-10;
                                if (monsters[index].y<-10) monsters[index].y=-10;
                                if (monsters[index].y>63) monsters[index].y=-10;
                                break;
                        case 11:
                                monsters[index].x = monsters[index].x + (monsters[index].directionX ? 0.75+(cycle*0.1) : -0.75+(cycle*0.1));
                                if (random(20)==10) monsters[index].directionX = random(2);
                                if (monsters[index].x <= 0)
                                {
                                        monsters[index].x = 0;
                                        monsters[index].directionX = !monsters[index].directionX;
                                }
                                if (monsters[index].x >= 117)
                                {
                                        monsters[index].x = 117;
                                        monsters[index].directionX = !monsters[index].directionX;
                                }
                                if (monsters[index].y >63)
                                {
                                        monsters[index].y = -8;
                                        monsters[index].state=10;
                                }
                                if ((monsters[index].y >=monsters[index].horizontalBarrier) && (monsters[index].state==10))
                                {
                                        monsters[index].timer = millis();
                                        monsters[index].state=11;

                                }
                                if (monsters[index].state==11 && (millis()-monsters[index].timer>(3000+random(3000)))) monsters[index].state=12;
                                if (monsters[index].state==10 || monsters[index].state==12) monsters[index].y += 0.3+(cycle*0.1);
                                break;
                        case 12:            // Polos - Move down left right in sequence

                                if (monsters[index].directionX==0 && monsters[index].directionY==1)
                                {
                                        monsters[index].y+=0.5+(cycle*0.1);
                                        if(random(25)==12)
                                        {
                                                monsters[index].directionX=1;
                                                monsters[index].directionY=0;
                                                break;
                                        }
                                }
                                if (monsters[index].directionX==1 && monsters[index].directionY==0)
                                {
                                        monsters[index].x+=0.75+(cycle*0.1);
                                        if(random(25)==12)
                                        {
                                                monsters[index].directionX=0;
                                                monsters[index].directionY=0;
                                                break;
                                        }
                                }
                                if (monsters[index].directionX==0 && monsters[index].directionY==0)
                                {
                                        monsters[index].x-=0.75+(cycle*0.1);
                                        if(random(25)==12)
                                        {
                                                monsters[index].directionX=0;
                                                monsters[index].directionY=1;
                                                break;
                                        }
                                }
                                if (monsters[index].x<0) monsters[index].x=0;
                                if (monsters[index].x>117) monsters[index].x=117;
                                if (monsters[index].y<-18) monsters[index].y=-18;
                                if (monsters[index].y>63) monsters[index].y=-10;
                                break;
                        }
                }
        }

        // toggle the monster animations
        int changeRate;
        switch(monstersIndex)
        {
        case 1:
        case 2:
        case 4:

                changeRate=500;
                break;
        case 3:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 12:
                changeRate=200;
                break;
        case 10:
                changeRate=50;
                break;
        }

        if ((millis()-monsterAnimRate)>changeRate)
        {

                for (int index=0; index<numMonsters; index++)
                {
                        if (monsters[index].state>0)
                        {
                                if (monstersIndex!=11) monsters[index].state++;
                                switch(monstersIndex)
                                {

                                case 1: // Arrows
                                case 2: // Butterflys
                                case 3: // UFOs
                                case 7: // Radars
                                        if (monsters[index].state > 2) monsters[index].state = 1;
                                        break;
                                case 4: // Whirlygigs
                                case 6: // Spinnybox
                                case 8: // Rockers
                                case 9: // Squiddys
                                        if (monsters[index].state > 4) monsters[index].state = 1;
                                        break;
                                case 5: // Birdies
                                        if (monsters[index].state > 5) monsters[index].state = 1;
                                        break;
                                case 10: // Bubbles
                                        if (monsters[index].state > 6) monsters[index].state = 1;
                                        break;
                                case 12: // Polos
                                        if (monsters[index].state > 3) monsters[index].state = 1;
                                        break;
                                }
                        }
                }
                monsterAnimRate=millis();
        }

}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void CheckLaserHit()
{
        for (int index1=0; index1<8; index1++)
        {
                for (int index2=0; index2<numMonsters; index2++)
                {
                        if (lasers[index1].age>0 && monsters[index2].state>0)
                        {
                                int topB, bottomB, leftB, rightB;
                                switch(monstersIndex)
                                {

                                case 2: // Butterfly
                                        topB = monsters[index2].y;
                                        leftB = monsters[index2].x;
                                        bottomB = monsters[index2].y+7;
                                        rightB = monsters[index2].x+11;
                                        break;
                                case 3: // UFOs
                                        topB = monsters[index2].y;
                                        leftB = monsters[index2].x;
                                        bottomB = monsters[index2].y+9;
                                        rightB = monsters[index2].x+10;
                                        break;
                                case 4: // Whirlygigs
                                        topB = monsters[index2].y;
                                        leftB = monsters[index2].x;
                                        bottomB = monsters[index2].y+7;
                                        rightB = monsters[index2].x+4;
                                        break;
                                case 1: // Arrows
                                case 5: // Birdies
                                        topB = monsters[index2].y;
                                        leftB = monsters[index2].x;
                                        bottomB = monsters[index2].y+7;
                                        rightB = monsters[index2].x+9;
                                        break;
                                case 6: // Spinnybox
                                case 7: // Radars
                                case 10: // Bubbles
                                case 9: // Squiddies
                                case 12: // Polos
                                        topB = monsters[index2].y;
                                        leftB = monsters[index2].x;
                                        bottomB = monsters[index2].y+9;
                                        rightB = monsters[index2].x+9;
                                        break;
                                case 8: // Rockers
                                        topB = monsters[index2].y;
                                        leftB = monsters[index2].x;
                                        bottomB = monsters[index2].y+10;
                                        rightB = monsters[index2].x+7;
                                        break;
                                case 11: // Wings
                                        topB = monsters[index2].y;
                                        leftB = monsters[index2].x;
                                        bottomB = monsters[index2].y+4;
                                        rightB = monsters[index2].x+9;
                                        break;
                                }

                                int topA = lasers[index1].y;
                                int bottomA = lasers[index1].y+3;
                                int leftA = lasers[index1].x;
                                int rightA = lasers[index1].x;

                                if (topA <= bottomB && bottomA >= topB && leftA <= rightB && rightA >= leftB)
                                {
                                        monsters[index2].state = 0;
                                        lasers[index1].age = 0;
                                        score=score+level;
                                        sprites.drawSelfMasked( leftA-4, topA-4, POP, 1);
                                }
                        }
                }
        }
        arduboy.display();
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void CheckShipHit()
{
        for (int index=0; index<numMonsters; index++)
        {
                if (monsters[index].state>0)
                {
                        int topA = shipY+6;
                        int bottomA = shipY+15;
                        int leftA = shipX;
                        int rightA = shipX+10;

                        int topB, bottomB, leftB, rightB;
                        switch(monstersIndex)
                        {
                        case 2: // Butterflys
                                topB = monsters[index].y;
                                leftB = monsters[index].x;
                                bottomB = monsters[index].y+7;
                                rightB = monsters[index].x+11;
                                break;
                        case 3: // UFOs
                                topB = monsters[index].y;
                                leftB = monsters[index].x;
                                bottomB = monsters[index].y+9;
                                rightB = monsters[index].x+10;
                                break;
                        case 4: // Whirlygigs
                                topB = monsters[index].y;
                                leftB = monsters[index].x;
                                bottomB = monsters[index].y+7;
                                rightB = monsters[index].x+4;
                                break;
                        case 1:                         // Arrows
                        case 5:                         // Birdies
                                topB = monsters[index].y;
                                leftB = monsters[index].x;
                                bottomB = monsters[index].y+7;
                                rightB = monsters[index].x+9;
                                break;
                        case 8: // Rockers
                                topB = monsters[index].y;
                                leftB = monsters[index].x;
                                bottomB = monsters[index].y+10;
                                rightB = monsters[index].x+7;
                                break;
                        case 6: // Spinnybox
                        case 7: // Radars
                        case 10: // Bubbles
                        case 9: // Squiddies
                        case 12: // Polos
                                topB = monsters[index].y;
                                leftB = monsters[index].x;
                                bottomB = monsters[index].y+9;
                                rightB = monsters[index].x+9;
                                break;
                        case 11: // Wings
                                topB = monsters[index].y;
                                leftB = monsters[index].x;
                                bottomB = monsters[index].y+4;
                                rightB = monsters[index].x+9;
                                break;
                        }


                        if (topA <= bottomB && bottomA >= topB && leftA <= rightB && rightA >= leftB)
                        {
                                shipExplodes();
                        }
                }
        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void shipExplodes()
{
        struct particles
        {
                byte x;
                byte y;
                byte direction;
        };

        particles bang[25];
        particles bangCopy[25];

        for (int index=0; index<25; index++)
        {
                bang[index].x = (shipX-5)+random(20);
                bang[index].y = shipY+random(20);
                bang[index].direction = random(8);
                bangCopy[index].x =   bang[index].x;
                bangCopy[index].y =   bang[index].y;
                bangCopy[index].direction =   bang[index].direction;
        }

        long timer = millis();

        while((millis()-timer)<3000)
        {
                for (int index=0; index<25; index++)
                {
                        arduboy.drawPixel(bangCopy[index].x, bangCopy[index].y, BLACK);
                        switch(bang[index].direction)
                        {
                        case 0:
                                bang[index].y=bang[index].y - 1;
                                break;
                        case 1:
                                bang[index].y=bang[index].y - 1;
                                bang[index].x=bang[index].x+1;
                                break;
                        case 2:
                                bang[index].x=bang[index].x+1;
                                break;
                        case 3:
                                bang[index].y=bang[index].y + 1;
                                bang[index].x=bang[index].x + 1;
                                break;
                        case 4:
                                bang[index].y=bang[index].y + 1;
                                break;
                        case 5:
                                bang[index].y=bang[index].y + 1;
                                bang[index].x=bang[index].x - 1;
                                break;
                        case 6:
                                bang[index].x=bang[index].x - 1;
                                break;
                        case 7:
                                bang[index].y=bang[index].y - 1;
                                bang[index].x=bang[index].x - 1;
                                break;
                        }
                        arduboy.drawPixel(bang[index].x, bang[index].y, WHITE);
                        arduboy.display();
                        //Serial.write(arduboy.getBuffer(), 128 * 64 / 8);
                        bangCopy[index].x =   bang[index].x;
                        bangCopy[index].y =   bang[index].y;
                        bangCopy[index].direction =   bang[index].direction;
                        digitalWrite(2, HIGH); //positive square wave
                        digitalWrite(5, LOW);              //positive square wavedelayMicroseconds(start);      //192uS
                        delayMicroseconds(millis()-timer);
                        digitalWrite(2, LOW);              //neutral square wave
                        digitalWrite(5, HIGH);              //positive square wave
                        delayMicroseconds(millis()-timer);              //192uS}
                }
        }
        lives=lives-1;
        if (lives>=0) levelStart();
        else gameOver();
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void checkIfAllMonstersDead()
{
        byte total=0;
        for (int index=0; index<numMonsters; index++)
        {
                if (monsters[index].state>0) total++;
        }
        if (total==0) initialiseMonsters();
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void nextLevel()
{
        level++;
        levelTimer=60;
        monstersIndex++;

        if (monstersIndex>maxNumMonsters)
        {
                monstersIndex=1;
                numMonsters++;
                cycle++;
        }
        levelStart();
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void levelStart()
{
        EEPROM.get(SAVELOCATION, highScore);
        initialiseLasers();
        initialiseMonsters();
        shipX=58;
        shipY=47;
        arduboy.clear();

        // Print score
        String buffer = String("Score: ");
        buffer += score;
        int widthInPixels = buffer.length() * 6 - 1;
        arduboy.setCursor(64-(widthInPixels/2),10);
        arduboy.print(buffer);

// print level
        buffer = String("Level: ");
        buffer += level;
        widthInPixels = buffer.length() * 6 - 1;
        arduboy.setCursor(64-(widthInPixels/2),24);
        arduboy.print(buffer);

        for (int index=0; index<lives; index++)
        {
                sprites.drawSelfMasked((65-(16*lives)/2)+(index*16), 36, SHIP, 0);
        }
        //Serial.write(arduboy.getBuffer(), 128 * 64 / 8);
        arduboy.display();
        delay(2000);
        shipLand();
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void gameOver()
{

        if (score > highScore) EEPROM.put(SAVELOCATION, score); delay(100);
        arduboy.clear();
        arduboy.setTextSize(2);
        arduboy.setCursor(8, 40);
        arduboy.print("GAME OVER ");
        arduboy.setTextSize(1);

        arduboy.setCursor(22, 2);
        if (score > highScore) arduboy.print("NEW HIGH SCORE");
        else
        {
                arduboy.setCursor(48, 2);
                arduboy.print("SCORE");
        }
        byte len = floor (log10 (abs (score))) + 1;
        arduboy.setCursor(60-(2*len),18);
        arduboy.print(score);
        arduboy.display();
        //Serial.write(arduboy.getBuffer(), 128 * 64 / 8);
        delay(5000);
        lives=4;
        level=1;
        monstersIndex=1;
        score=0;
        numMonsters=6;
        levelTimer=100;
        levelStart();
        arduboy.setTextSize(1);
}

void eraseEEPROM()
{
        arduboy.clear();
        boolean up = false;

        while (1)
        {
                if( arduboy.pressed(UP_BUTTON) == true ) {
                        up = true;
                        arduboy.clear();
                }

                if( arduboy.pressed(DOWN_BUTTON) == true) {
                        up = false;
                        arduboy.clear();
                }

                if (up == true)
                {
                        arduboy.fillCircle(13,23,3);
                }
                else
                {
                        arduboy.fillCircle(13,43,3);
                }

                if( arduboy.pressed(A_BUTTON) == true ) {
                        if (up == true)
                        {
                                delay(150);
                                highScore = 0;
                                EEPROM.put(SAVELOCATION, highScore);
                                break;
                        }
                        else
                        {
                                delay(150);
                                break;
                        }
                }
                arduboy.setCursor(20,00);
                arduboy.print("R U SURE?");
                arduboy.setCursor(20,20);
                arduboy.print("Y");
                arduboy.setCursor(20,40);
                arduboy.print("N");
                arduboy.display();
        }

}

void menu()
{
        boolean up = true;
        arduboy.clear();
        while (1)
        {
          arduboy.setTextSize(2);
          arduboy.setCursor(24,4);
          arduboy.print("ARCODIA");
          arduboy.setTextSize(1);
          if( arduboy.pressed(UP_BUTTON) == true ) {
                        up = true;
                        arduboy.clear();
                }

                if( arduboy.pressed(DOWN_BUTTON) == true) {
                        up = false;
                        arduboy.clear();
                }

                if (up == true)
                {
                        arduboy.fillCircle(13,30,3);
                }
                else
                {
                        arduboy.fillCircle(13,50,3);
                }

                if( arduboy.pressed(A_BUTTON) == true ) {
                    if (up == true)
                    {
                      delay(150);
                      break;
                    }
                    else
                    {
                      delay(150);
                      eraseEEPROM();
                      levelStart();
                      break;
                    }
                }
                arduboy.setCursor(20,28);
                arduboy.print("NEW GAME");
                arduboy.setCursor(20,48);
                arduboy.print("RESET HI-SCORE");
                arduboy.display();
        }
}

// #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=
void loop() {
        if (!arduboy.nextFrame()) return;  // Keep frame rate at 60fps
        arduboy.clear();
        arduboy.drawFastHLine(0,0,2.133*levelTimer);
        checkButtons();
        moveThings();
        drawShip();
        drawMonsters();
        drawlasers();
        CheckLaserHit();
        CheckShipHit();
        //Serial.write(arduboy.getBuffer(), 128 * 64 / 8);
        arduboy.display();
        checkIfAllMonstersDead();
        if ((millis()-tick)>1000)
        {
                levelTimer--;
                tick=millis();
                if (levelTimer==0) nextLevel();
        }
}
