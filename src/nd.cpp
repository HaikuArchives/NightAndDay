/*
Night and Day : Automatic desktop color changer
Copyright (C) 1998-1999 Jean-Baptiste M. Queru

This program is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License 
as published by the Free Software Foundation; either version 2 
of the License, or (at your option) any later version. 

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
GNU General Public License for more details. 

You should have received a copy of the GNU General Public License 
along with this program; if not, write to the Free Software 
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Contact the author at:
Jean-Baptiste M. Queru, 1706 Marina Ct #B, San Mateo, CA 94403, USA
or by e-mail at : djaybee@cyberdude.com
*/

#include "nd.h"
#include <Application.h>
#include <Window.h>
#include <View.h>
#include <Box.h>
#include <Control.h>
#include <RadioButton.h>
#include <StringView.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <Path.h>
#include <Bitmap.h>
#include <Alert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <File.h>
#include <Screen.h>
#include <FindDirectory.h>

const int numlang=3;

char* text [][numlang]={
{"Night and Day","Jour et Nuit","Dag en Nacht"},
{"2 colors","2 couleurs","2 kleuren"},
{"3 colors","3 couleurs","3 kleuren"},
{"4 colors","4 couleurs","4 kleuren"},
{"midnight","minuit","nacht"},
{"noon","midi","middag"},
{"6:00","6 heures","6 uur"},
{"sunrise","matin","morgen"},
{"sunset","soir","avond"},

{"About…","À propos…","Over…"},

{"Language","Langue","Taal"},
{"Save","Sauvegarder","Opslaan"},
{"Quit","Quitter","Afsluiten"},
{"About Night and Day","À Propos de Jour et Nuit","Over Dag en Nacht"},

{"Night and Day version 0.1.2\n©1998 Jean-Baptiste \"Djaybee\" Quéru\n<djaybee@cyberdude.com>\n\n"
"Night and Day comes with ABSOLUTELY NO WARRANTY. For details see the \"ReadMe\" file",
"Jour et Nuit version 0.1.2\n©1998 Jean-Baptiste \"Djaybee\" Quéru\n<djaybee@cyberdude.com>\n\n"
"Jour et Nuit ne vient avec ABSOLUMENT AUCUNE GARANTIE. Pour plus de details voir le fichier \"LisezMoi\"",
"Dag en Nacht versie 0.1.2\n©1998 Jeann-Baptiste \"Djaybee\" Quéru\n<djaybee@cyberdude.com>\n\n"
"Dag en Nacht komt ABSOLUUT ZONDER GARANTIE. Kijk voor meer details in het \"ReadMe\" bestand"},

{"OK","Fermer","OK"},
{"Directory not found","Repertoire non trouvé","Map niet gevonden"},
{"Settings directory not found\nCannot save preferences",
"Repertoire de préférences non trouvé\nImpossible de sauver les réglages",
"De map met instellingen is niet gevonden\nKan de voorkeuren niet opslaan"},
{"Too Bad","Dommage","Helaas"},
{"File write error","Erreur écriture de fichier","Fout bij het schrijven van bestand"},
{"Error while trying to save preferences","Erreur en essayant de sauver les réglages","Fout terwijl voorkeuren worden opgeslagen"}
};

char* langname[numlang][numlang]={
{"English","Anglais","Engels"},
{"French","Français","Frans"},
{"Dutch", "Hollandais","Nederlands"}
};

class DKApplication;
class DKMainWindow;
class DKColorControl;
class DKScreenView;

struct {
	DKMainWindow *window0;
	BRadioButton *button1,*button2,*button3;
	BStringView *sview4a,*sview4b,*sview4c,*sview5a,*sview5b,*sview5c,*sview6,*sview7,*sview8;
	BMenuItem *mitem9,*mitem10,*mitem11,*mitem12;
	BMenu *submenu;
} langitem;

class DKApplication:public BApplication {
public:
	DKApplication();
private:
	void ReadyToRun();
	bool QuitRequested();
	void AboutRequested();
	void MessageReceived(BMessage*);
	void SaveSettings(bool silent=true);
	bool LoadSettings();
	thread_id ctid,btid;
};

class DKMainWindow:public BWindow {
public:
	DKMainWindow();

private:
	bool QuitRequested();
	void MessageReceived(BMessage*);
	void ScreenChanged(BRect,color_space);

	BView* backview;
	DKColorControl *cc [27];
};

class DKColorControl:public BControl {
public:
	DKColorControl(BPoint topleft,int numcolor);
	void SetValue(int32);
	thread_id tid;
private:
	void Draw(BRect);
	void MouseDown(BPoint);
	void KeyDown(const char*,int32);
	void AttachedToWindow();
	void DetachedFromWindow();

	int numcolor;
};

class DKScreenView:public BView {
public:
	DKScreenView();
private:
	void AttachedToWindow();
	void Draw(BRect);
	void MouseDown(BPoint p);

	BPopUpMenu* menu;
};

class DKPreview:public BView {
public:
	DKPreview();
private:
	void Draw(BRect);
};

long dkthread(void*);
long mousethread(void*);
long bitmapthread(void*);

const rgb_color light_gray={255,255,255,0},
			medium_gray={224,224,224,0},
			dark_gray={112,112,112,0},
			black={0,0,0,0},
			white={255,255,255,0};

BBitmap* ColorRamp[3][4];
DKScreenView* scrview;
DKPreview* preview;
sem_id ccsem;
sem_id mpsem;
sem_id bdsem;
sem_id bpsem;
sem_id blsem;
volatile bool data_avail=true,thr_running=false;
BBitmap* PreviewRamp[4];
volatile int ticks=0;
volatile int internaldepth;


int32 language=0;
rgb_color c0={0,0,0,0},c6={0,0,255,0},c12={224,224,255,0},c18={0,0,255,0};
int mode=3;

DKApplication::DKApplication():BApplication("application/x-vnd.djaybee-deskcol") {
	Run();
}

void DKApplication::ReadyToRun() {
	if (!LoadSettings()) {
		mode=3;
		language=0;
		c0.red=c0.green=c0.blue=c6.red=c6.green=c18.red=c18.green=0;
		c6.blue=c12.blue=c18.blue=255;
		c12.red=c12.green=224;
	}
	ccsem=create_sem(0,"Color Changer sem");
	mpsem=create_sem(1,"Mouse Poller sem");
	bdsem=create_sem(1,"Bitmap Data access");
	blsem=create_sem(0,"Bitmap thread locker");
	bpsem=create_sem(1,"Bitmap lock protector");
	resume_thread(ctid=spawn_thread(dkthread,"Color Changer",B_NORMAL_PRIORITY,NULL));
	resume_thread(btid=spawn_thread(bitmapthread,"Bitmap Drawer",B_NORMAL_PRIORITY,NULL));
	switch(mode) {
		case 2:
			c6.red=c18.red=(c0.red+c12.red)/2;
			c6.green=c18.green=(c0.green+c12.green)/2;
			c6.blue=c18.blue=(c0.blue+c12.blue)/2;
			break;
		case 3:
			c6.red=c18.red=(c6.red+c18.red)/2;
			c6.green=c18.green=(c6.green+c18.green)/2;
			c6.blue=c18.blue=(c6.blue+c18.blue)/2;
			break;
	}
	langitem.window0=new DKMainWindow;
}

bool DKApplication::QuitRequested() {
	acquire_sem(ccsem);
	kill_thread(ctid);
	release_sem(ccsem);
	acquire_sem(bpsem);
	kill_thread(btid);
	release_sem(bpsem);
	SaveSettings(true);
	return true;
}

void DKApplication::MessageReceived(BMessage* m) {
	switch(m->what) {
		case 'lang' : {
			int32 nl;
			if ((m->FindInt32("lang",&nl)==B_OK)&&(nl>=0)&&(nl<numlang)&&(nl!=language)) {
				langitem.window0->SetTitle(text[0][nl]);
				langitem.window0->Lock();
				langitem.button1->SetLabel(text[1][nl]);
				langitem.button2->SetLabel(text[2][nl]);
				langitem.button3->SetLabel(text[3][nl]);
				langitem.sview4a->SetText(text[4][nl]);
				langitem.sview4b->SetText(text[4][nl]);
				langitem.sview4c->SetText(text[4][nl]);
				langitem.sview5a->SetText(text[5][nl]);
				langitem.sview5b->SetText(text[5][nl]);
				langitem.sview5c->SetText(text[5][nl]);
				langitem.sview6->SetText(text[6][nl]);
				langitem.sview7->SetText(text[7][nl]);
				langitem.sview8->SetText(text[8][nl]);
				langitem.window0->Unlock();
				langitem.mitem9->SetLabel(text[9][nl]);
				langitem.mitem10->SetLabel(text[10][nl]);
				langitem.mitem11->SetLabel(text[11][nl]);
				langitem.mitem12->SetLabel(text[12][nl]);
				for (int i=0;i<numlang;i++) {
					langitem.submenu->ItemAt(i)->SetLabel(langname[i][nl]);
					langitem.submenu->ItemAt(i)->SetMarked(nl==i);
				}
				language=nl;
			}
			break;
		}
		case 'save' : {
			SaveSettings(false);
			break;
		}
		default : {
			BApplication::MessageReceived(m);
			break;
		}
	}
}

void DKApplication::SaveSettings(bool silent) {
	BPath path;
	char buff[129];
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path)<B_OK) {
		if (!silent) {
			(new BAlert(text[16][language],text[17][language],text[18][language]))->Go(NULL);
		}
		return;
	}
	path.Append("Night and Day settings");
	BFile file(path.Path(),B_WRITE_ONLY|B_CREATE_FILE|B_ERASE_FILE);
	if (!file.IsWritable()) {
		if (!silent) {
			(new BAlert(text[19][language],text[20][language],text[18][language]))->Go(NULL);
		}
		return;
	}
	sprintf(buff,"# mode : number of colors\n%d\n",mode);
	if (file.Write(buff,strlen(buff))<B_OK) {
		if (!silent) {
			(new BAlert(text[19][language],text[20][language],text[18][language]))->Go(NULL);
		}
		return;
	}
	sprintf(buff,"# language : 0=English, 1=French\n%ld\n",language);
	if (file.Write(buff,strlen(buff))<B_OK) {
		if (!silent) {
			(new BAlert(text[19][language],text[20][language],text[18][language]))->Go(NULL);
		}
		return;
	}
	sprintf(buff,"# color for midnight\n%d\n%d\n%d\n",c0.red,c0.green,c0.blue);
	if (file.Write(buff,strlen(buff))<B_OK) {
		if (!silent) {
			(new BAlert(text[19][language],text[20][language],text[18][language]))->Go(NULL);
		}
		return;
	}
	sprintf(buff,"# color for 6AM\n%d\n%d\n%d\n",c6.red,c6.green,c6.blue);
	if (file.Write(buff,strlen(buff))<B_OK) {
		if (!silent) {
			(new BAlert(text[19][language],text[20][language],text[18][language]))->Go(NULL);
		}
		return;
	}
	sprintf(buff,"# color for noon\n%d\n%d\n%d\n",c12.red,c12.green,c12.blue);
	if (file.Write(buff,strlen(buff))<B_OK) {
		if (!silent) {
			(new BAlert(text[19][language],text[20][language],text[18][language]))->Go(NULL);
		}
		return;
	}
	sprintf(buff,"# color for 6PM\n%d\n%d\n%d\n",c18.red,c18.green,c18.blue);
	if (file.Write(buff,strlen(buff))<B_OK) {
		if (!silent) {
			(new BAlert(text[19][language],text[20][language],text[18][language]))->Go(NULL);
		}
		return;
	}
}

bool DKApplication::LoadSettings() {
	BPath path;
	char buff[129];
	int offset=0;
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path)<B_OK) {
		return false;
	}
	path.Append("Night and Day settings");
	BFile file(path.Path(),B_READ_ONLY);
	if (!file.IsReadable()) {
		return false;
	}
	for (int n=0;n<14;n++) {
		do {
			if (file.ReadAt(offset,buff,129)<B_OK) {
				return false;
			}
			if (buff[0]=='#') {
				offset+=(strchr(buff,'\n'))-buff+1;
			}
		} while (buff[0]=='#');
		int vr;
		if (sscanf(buff,"%d",&vr)<1) {
			return false;
		}
		offset+=(strchr(buff,'\n'))-buff+1;
		if ((vr<0)||(vr>255)) return false;
		switch (n) {
			case 0:
				if ((vr<2)||(vr>4)) return false;
				mode=vr;
				break;
			case 1:
				if ((vr<0)||(vr>=numlang)) return false;
				language=vr;
				break;
			case 2:
				c0.red=vr;
				break;
			case 3:
				c0.green=vr;
				break;
			case 4:
				c0.blue=vr;
				break;
			case 5:
				c6.red=vr;
				break;
			case 6:
				c6.green=vr;
				break;
			case 7:
				c6.blue=vr;
				break;
			case 8:
				c12.red=vr;
				break;
			case 9:
				c12.green=vr;
				break;
			case 10:
				c12.blue=vr;
				break;
			case 11:
				c18.red=vr;
				break;
			case 12:
				c18.green=vr;
				break;
			case 13:
				c18.blue=vr;
				break;
		}
	}
	return true;
}

void DKApplication::AboutRequested() {
	(new BAlert(text[13][language],text[14][language],text[15][language]))->Go(NULL);
}

DKMainWindow::DKMainWindow():BWindow(BRect(100,100,471,365),text[0][language],B_TITLED_WINDOW_LOOK,B_NORMAL_WINDOW_FEEL,B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_NOT_ANCHORED_ON_ACTIVATE) {
	for (int i=0;i<3;i++) {
		ColorRamp[i][0]=new BBitmap(BRect(0,0,11,127),B_CMAP8);
		ColorRamp[i][1]=new BBitmap(BRect(0,0,11,127),B_RGB15);
		ColorRamp[i][2]=new BBitmap(BRect(0,0,11,127),B_RGB16);
		ColorRamp[i][3]=new BBitmap(BRect(0,0,11,127),B_RGB32);
	}

	PreviewRamp[0]=new BBitmap(BRect(0,0,337,14),B_CMAP8);
	PreviewRamp[1]=new BBitmap(BRect(0,0,337,14),B_RGB15);
	PreviewRamp[2]=new BBitmap(BRect(0,0,337,14),B_RGB16);
	PreviewRamp[3]=new BBitmap(BRect(0,0,337,14),B_RGB32);

	uchar* fb;
	int ll;
	
	for (int n=0;n<3;n++) {
		fb=(uchar*)ColorRamp[n][0]->Bits();
		ll=ColorRamp[n][0]->BytesPerRow();
		for (int j=0;j<128;j++) {
			for (int i=0;i<12;i++) {
				float c=10.-j/12.75;
				uchar cc=int(c);
				if (float(rand())/RAND_MAX<c-cc) cc++;
				if (cc>0) cc=52+10*n-cc-((n==2)?30:0);
				fb[i+ll*j]=cc;
			}
		}
	}
	for (int n=0;n<3;n++) {
		fb=(uchar*)ColorRamp[n][1]->Bits();
		ll=ColorRamp[n][1]->BytesPerRow();
		for (int j=0;j<128;j++) {
			for (int i=0;i<12;i++) {
				float c=float(255-2*j)*31/255;
				uchar cc=int(c);
				if (float(rand())/RAND_MAX<c-cc) cc++;
				*(ushort*)(fb+i*2+ll*j)=B_HOST_TO_LENDIAN_INT16(cc<<(5*(2-n)));
			}
		}
	}
	for (int n=0;n<3;n++) {
		fb=(uchar*)ColorRamp[n][2]->Bits();
		ll=ColorRamp[n][2]->BytesPerRow();
		for (int j=0;j<128;j++) {
			for (int i=0;i<12;i++) {
				float c;
				if (n==1) {
					c=float(255-2*j)*63/255;
				} else {
					c=float(255-2*j)*31/255;
				}
				uchar cc=int(c);
				if (float(rand())/RAND_MAX<c-cc) cc++;
				*(ushort*)(fb+i*2+ll*j)=B_HOST_TO_LENDIAN_INT16(cc<<(5*(2-n)+((n==0)?1:0)));
			}
		}
	}
	for (int n=0;n<3;n++) {
		fb=(uchar*)ColorRamp[n][3]->Bits();
		ll=ColorRamp[n][3]->BytesPerRow();
		for (int j=0;j<128;j++) {
			for (int i=0;i<12;i++) {
				for (int c=0;c<3;c++) {
					fb[4*i+c+ll*j]=(c+n==2)?255-2*j:0;
				}
			}
		}
	}


	BView* view;
	BBox* box;
	BRadioButton* rbut;
	BMessage* mess;
	BStringView* sview;
	view=new BView(BRect(0,0,371,265),"",B_FOLLOW_NONE,B_WILL_DRAW);
	view->SetViewColor(medium_gray);
	AddChild(view);

	box=new BBox(BRect(10,10,316,38));
	view->AddChild(box);

	mess=new BMessage('ncol');
	mess->AddInt32("colors",2);
	langitem.button1=rbut=new BRadioButton(BRect(7,6,98,6),"",text[1][language],mess);
	if (mode==2) rbut->SetValue(1);
	box->AddChild(rbut);

	mess=new BMessage('ncol');
	mess->AddInt32("colors",3);
	langitem.button2=rbut=new BRadioButton(BRect(107,6,198,6),"",text[2][language],mess);
	if (mode==3) rbut->SetValue(1);
	box->AddChild(rbut);

	mess=new BMessage('ncol');
	mess->AddInt32("colors",4);
	langitem.button3=rbut=new BRadioButton(BRect(207,6,298,6),"",text[3][language],mess);
	if (mode==4) rbut->SetValue(1);
	box->AddChild(rbut);

	scrview=new DKScreenView();
	view->AddChild(scrview);

	backview=new BView(BRect(0,39,1115,226),"",B_FOLLOW_ALL_SIDES,B_WILL_DRAW);
	backview->SetViewColor(medium_gray);
	view->AddChild(backview);
	backview->ScrollTo(372*(mode-2),0);

	box=new BBox(BRect(10,10,361,177));
	backview->AddChild(box);

	cc[0]=new DKColorControl(BPoint(104,12),0);
	cc[0]->SetValue(c0.red);
	box->AddChild(cc[0]);
	cc[1]=new DKColorControl(BPoint(122,12),1);
	cc[1]->SetValue(c0.green);
	box->AddChild(cc[1]);
	cc[2]=new DKColorControl(BPoint(140,12),2);
	cc[2]->SetValue(c0.blue);
	box->AddChild(cc[2]);

	langitem.sview4a=sview=new BStringView(BRect(104,150,155,161),"",text[4][language]);
	sview->SetAlignment(B_ALIGN_CENTER);
	box->AddChild(sview);

	cc[3]=new DKColorControl(BPoint(196,12),3);
	cc[3]->SetValue(c12.red);
	box->AddChild(cc[3]);
	cc[4]=new DKColorControl(BPoint(214,12),4);
	cc[4]->SetValue(c12.green);
	box->AddChild(cc[4]);
	cc[5]=new DKColorControl(BPoint(232,12),5);
	cc[5]->SetValue(c12.blue);
	box->AddChild(cc[5]);

	langitem.sview5a=sview=new BStringView(BRect(196,150,247,161),"",text[5][language]);
	sview->SetAlignment(B_ALIGN_CENTER);
	box->AddChild(sview);

	box=new BBox(BRect(382,10,733,177));
	backview->AddChild(box);

	cc[6]=new DKColorControl(BPoint(58,12),6);
	cc[6]->SetValue(c0.red);
	box->AddChild(cc[6]);
	cc[7]=new DKColorControl(BPoint(76,12),7);
	cc[7]->SetValue(c0.green);
	box->AddChild(cc[7]);
	cc[8]=new DKColorControl(BPoint(94,12),8);
	cc[8]->SetValue(c0.blue);
	box->AddChild(cc[8]);

	langitem.sview4b=sview=new BStringView(BRect(58,150,109,161),"",text[4][language]);
	sview->SetAlignment(B_ALIGN_CENTER);
	box->AddChild(sview);

	cc[9]=new DKColorControl(BPoint(150,12),9);
	cc[9]->SetValue(c6.red);
	box->AddChild(cc[9]);
	cc[10]=new DKColorControl(BPoint(168,12),10);
	cc[10]->SetValue(c6.green);
	box->AddChild(cc[10]);
	cc[11]=new DKColorControl(BPoint(186,12),11);
	cc[11]->SetValue(c6.blue);
	box->AddChild(cc[11]);

	langitem.sview6=sview=new BStringView(BRect(150,150,201,161),"",text[6][language]);
	sview->SetAlignment(B_ALIGN_CENTER);
	box->AddChild(sview);

	cc[12]=new DKColorControl(BPoint(242,12),12);
	cc[12]->SetValue(c12.red);
	box->AddChild(cc[12]);
	cc[13]=new DKColorControl(BPoint(260,12),13);
	cc[13]->SetValue(c12.green);
	box->AddChild(cc[13]);
	cc[14]=new DKColorControl(BPoint(278,12),14);
	cc[14]->SetValue(c12.blue);
	box->AddChild(cc[14]);

	langitem.sview5b=sview=new BStringView(BRect(242,150,293,161),"",text[5][language]);
	sview->SetAlignment(B_ALIGN_CENTER);
	box->AddChild(sview);

	box=new BBox(BRect(754,10,1105,177));
	backview->AddChild(box);

	cc[15]=new DKColorControl(BPoint(12,12),15);
	cc[15]->SetValue(c0.red);
	box->AddChild(cc[15]);
	cc[16]=new DKColorControl(BPoint(30,12),16);
	cc[16]->SetValue(c0.green);
	box->AddChild(cc[16]);
	cc[17]=new DKColorControl(BPoint(48,12),17);
	cc[17]->SetValue(c0.blue);
	box->AddChild(cc[17]);

	langitem.sview4c=sview=new BStringView(BRect(12,150,63,161),"",text[4][language]);
	sview->SetAlignment(B_ALIGN_CENTER);
	box->AddChild(sview);

	cc[18]=new DKColorControl(BPoint(104,12),18);
	cc[18]->SetValue(c6.red);
	box->AddChild(cc[18]);
	cc[19]=new DKColorControl(BPoint(122,12),19);
	cc[19]->SetValue(c6.green);
	box->AddChild(cc[19]);
	cc[20]=new DKColorControl(BPoint(140,12),20);
	cc[20]->SetValue(c6.blue);
	box->AddChild(cc[20]);

	langitem.sview7=sview=new BStringView(BRect(104,150,155,161),"",text[7][language]);
	sview->SetAlignment(B_ALIGN_CENTER);
	box->AddChild(sview);

	cc[21]=new DKColorControl(BPoint(196,12),21);
	cc[21]->SetValue(c12.red);
	box->AddChild(cc[21]);
	cc[22]=new DKColorControl(BPoint(214,12),22);
	cc[22]->SetValue(c12.green);
	box->AddChild(cc[22]);
	cc[23]=new DKColorControl(BPoint(232,12),23);
	cc[23]->SetValue(c12.blue);
	box->AddChild(cc[23]);

	langitem.sview5c=sview=new BStringView(BRect(196,150,247,161),"",text[5][language]);
	sview->SetAlignment(B_ALIGN_CENTER);
	box->AddChild(sview);

	cc[24]=new DKColorControl(BPoint(288,12),24);
	cc[24]->SetValue(c18.red);
	box->AddChild(cc[24]);
	cc[25]=new DKColorControl(BPoint(306,12),25);
	cc[25]->SetValue(c18.green);
	box->AddChild(cc[25]);
	cc[26]=new DKColorControl(BPoint(324,12),26);
	cc[26]->SetValue(c18.blue);
	box->AddChild(cc[26]);

	for (int i=0;i<27;i++) {
		cc[i]->SetEnabled((i>=3*(mode+1)*(mode-2)/2)&&(i<3*(mode-1)*(mode+2)/2));
	}

	langitem.sview8=sview=new BStringView(BRect(288,150,339,161),"",text[8][language]);
	sview->SetAlignment(B_ALIGN_CENTER);
	box->AddChild(sview);

	box=new BBox(BRect(10,227,361,255));
	view->AddChild(box);
	preview=new DKPreview();
	box->AddChild(preview);

	switch(BScreen(this).ColorSpace()) {
		case B_CMAP8 :
			internaldepth=0;
			break;
		case B_RGB15:
		case B_RGBA15:
		case B_RGB15_BIG:
		case B_RGBA15_BIG:
			internaldepth=1;
			break;
		case B_RGB16:
		case B_RGB16_BIG:
			internaldepth=2;
			break;
		default:
			internaldepth=3;
	}

	release_sem(ccsem);

	acquire_sem(bdsem);
	data_avail=true;
	release_sem(bdsem);
	release_sem(blsem);

	Show();
}

void DKMainWindow::MessageReceived(BMessage* m) {
	int32 n;
	switch (m->what) {
		case 'ncol' :
			if ((m->FindInt32("colors",&n)==B_OK)&&(n>=2)&&(n<=4)&&(mode!=n)) {
				acquire_sem(bdsem);
				for (int i=0;i<27;i++) {
					cc[i]->SetEnabled((i>=3*(n+1)*(n-2)/2)&&(i<3*(n-1)*(n+2)/2));
				}
				switch(n) {
					case 2:
						c6.red=c18.red=(c0.red+c12.red)/2;
						c6.green=c18.green=(c0.green+c12.green)/2;
						c6.blue=c18.blue=(c0.blue+c12.blue)/2;
						cc[9]->BControl::SetValue(c6.red);
						cc[18]->BControl::SetValue(c6.red);
						cc[24]->BControl::SetValue(c18.red);
						cc[10]->BControl::SetValue(c6.green);
						cc[19]->BControl::SetValue(c6.green);
						cc[25]->BControl::SetValue(c18.green);
						cc[11]->BControl::SetValue(c6.blue);
						cc[20]->BControl::SetValue(c6.blue);
						cc[26]->BControl::SetValue(c18.blue);
						ticks=0;
						break;
					case 3:
						if (mode==4) {
							c6.red=c18.red=(c6.red+c18.red)/2;
							c6.green=c18.green=(c6.green+c18.green)/2;
							c6.blue=c18.blue=(c6.blue+c18.blue)/2;
							cc[9]->BControl::SetValue(c6.red);
							cc[18]->BControl::SetValue(c6.red);
							cc[24]->BControl::SetValue(c18.red);
							cc[10]->BControl::SetValue(c6.green);
							cc[19]->BControl::SetValue(c6.green);
							cc[25]->BControl::SetValue(c18.green);
							cc[11]->BControl::SetValue(c6.blue);
							cc[20]->BControl::SetValue(c6.blue);
							cc[26]->BControl::SetValue(c18.blue);
							ticks=0;
						}
						break;
				}
				mode=n;
				backview->ScrollTo(372*(n-2),0);
				data_avail=true;
				if (!thr_running) {
					release_sem(blsem);
				}
				release_sem(bdsem);
			}
			break;
		case 'colo' :
			if ((m->FindInt32("color",&n)==B_OK)&&(n>=0)&&(n<27)) {
				acquire_sem(bdsem);
				switch(n) {
					case 0:
						c0.red=cc[0]->Value();
						c6.red=(c0.red+c12.red)/2;
						c18.red=c6.red;
						cc[6]->BControl::SetValue(c0.red);
						cc[15]->BControl::SetValue(c0.red);
						cc[9]->BControl::SetValue(c6.red);
						cc[18]->BControl::SetValue(c6.red);
						cc[24]->BControl::SetValue(c18.red);
						break;
					case 3:
						c12.red=cc[3]->Value();
						c6.red=(c0.red+c12.red)/2;
						c18.red=c6.red;
						cc[12]->BControl::SetValue(c12.red);
						cc[21]->BControl::SetValue(c12.red);
						cc[9]->BControl::SetValue((c0.red+c12.red)/2);
						cc[18]->BControl::SetValue((c0.red+c12.red)/2);
						cc[24]->BControl::SetValue((c0.red+c12.red)/2);
						break;
					case 6:
						c0.red=cc[6]->Value();
						cc[0]->BControl::SetValue(c0.red);
						cc[15]->BControl::SetValue(c0.red);
						break;
					case 9:
						c6.red=cc[9]->Value();
						c18.red=c6.red;
						cc[18]->BControl::SetValue(c6.red);
						cc[24]->BControl::SetValue(c6.red);
						break;
					case 12:
						c12.red=cc[12]->Value();
						cc[3]->BControl::SetValue(c12.red);
						cc[21]->BControl::SetValue(c12.red);
						break;
					case 15:
						c0.red=cc[15]->Value();
						cc[0]->BControl::SetValue(c0.red);
						cc[6]->BControl::SetValue(c0.red);
						break;
					case 18:
						c6.red=cc[18]->Value();
						cc[9]->BControl::SetValue(c6.red);
						break;
					case 21:
						c12.red=cc[21]->Value();
						cc[3]->BControl::SetValue(c12.red);
						cc[12]->BControl::SetValue(c12.red);
						break;
					case 24:
						c18.red=cc[24]->Value();
						break;
					case 1:
						c0.green=cc[1]->Value();
						c6.green=(c0.green+c12.green)/2;
						c18.green=c6.green;
						cc[7]->BControl::SetValue(c0.green);
						cc[16]->BControl::SetValue(c0.green);
						cc[10]->BControl::SetValue(c6.green);
						cc[19]->BControl::SetValue(c6.green);
						cc[25]->BControl::SetValue(c18.green);
						break;
					case 4:
						c12.green=cc[4]->Value();
						c6.green=(c0.green+c12.green)/2;
						c18.green=c6.green;
						cc[13]->BControl::SetValue(c12.green);
						cc[22]->BControl::SetValue(c12.green);
						cc[10]->BControl::SetValue((c0.green+c12.green)/2);
						cc[19]->BControl::SetValue((c0.green+c12.green)/2);
						cc[25]->BControl::SetValue((c0.green+c12.green)/2);
						break;
					case 7:
						c0.green=cc[7]->Value();
						cc[1]->BControl::SetValue(c0.green);
						cc[16]->BControl::SetValue(c0.green);
						break;
					case 10:
						c6.green=cc[10]->Value();
						c18.green=c6.green;
						cc[19]->BControl::SetValue(c6.green);
						cc[25]->BControl::SetValue(c6.green);
						break;
					case 13:
						c12.green=cc[13]->Value();
						cc[4]->BControl::SetValue(c12.green);
						cc[22]->BControl::SetValue(c12.green);
						break;
					case 16:
						c0.green=cc[16]->Value();
						cc[1]->BControl::SetValue(c0.green);
						cc[7]->BControl::SetValue(c0.green);
						break;
					case 19:
						c6.green=cc[19]->Value();
						cc[10]->BControl::SetValue(c6.green);
						break;
					case 22:
						c12.green=cc[22]->Value();
						cc[4]->BControl::SetValue(c12.green);
						cc[13]->BControl::SetValue(c12.green);
						break;
					case 25:
						c18.green=cc[25]->Value();
						break;
					case 2:
						c0.blue=cc[2]->Value();
						c6.blue=(c0.blue+c12.blue)/2;
						c18.blue=c6.blue;
						cc[8]->BControl::SetValue(c0.blue);
						cc[17]->BControl::SetValue(c0.blue);
						cc[11]->BControl::SetValue(c6.blue);
						cc[20]->BControl::SetValue(c6.blue);
						cc[26]->BControl::SetValue(c18.blue);
						break;
					case 5:
						c12.blue=cc[5]->Value();
						c6.blue=(c0.blue+c12.blue)/2;
						c18.blue=c6.blue;
						cc[14]->BControl::SetValue(c12.blue);
						cc[23]->BControl::SetValue(c12.blue);
						cc[11]->BControl::SetValue((c0.blue+c12.blue)/2);
						cc[20]->BControl::SetValue((c0.blue+c12.blue)/2);
						cc[26]->BControl::SetValue((c0.blue+c12.blue)/2);
						break;
					case 8:
						c0.blue=cc[8]->Value();
						cc[2]->BControl::SetValue(c0.blue);
						cc[17]->BControl::SetValue(c0.blue);
						break;
					case 11:
						c6.blue=cc[11]->Value();
						c18.blue=c6.blue;
						cc[20]->BControl::SetValue(c6.blue);
						cc[26]->BControl::SetValue(c6.blue);
						break;
					case 14:
						c12.blue=cc[14]->Value();
						cc[5]->BControl::SetValue(c12.blue);
						cc[23]->BControl::SetValue(c12.blue);
						break;
					case 17:
						c0.blue=cc[17]->Value();
						cc[2]->BControl::SetValue(c0.blue);
						cc[8]->BControl::SetValue(c0.blue);
						break;
					case 20:
						c6.blue=cc[20]->Value();
						cc[11]->BControl::SetValue(c6.blue);
						break;
					case 23:
						c12.blue=cc[23]->Value();
						cc[5]->BControl::SetValue(c12.blue);
						cc[14]->BControl::SetValue(c12.blue);
						break;
					case 26:
						c18.blue=cc[26]->Value();
						break;
				}
				data_avail=true;
				if (!thr_running) {
					release_sem(blsem);
				}
				release_sem(bdsem);
			}
			break;
		default:
			BWindow::MessageReceived(m);
	}
}

bool DKMainWindow::QuitRequested() {
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

void DKMainWindow::ScreenChanged(BRect, color_space cs) {
	switch(cs) {
		case B_CMAP8 :
			internaldepth=0;
			break;
		case B_RGB15:
		case B_RGBA15:
		case B_RGB15_BIG:
		case B_RGBA15_BIG:
			internaldepth=1;
			break;
		case B_RGB16:
		case B_RGB16_BIG:
			internaldepth=2;
			break;
		default:
			internaldepth=3;
	}
}

DKColorControl::DKColorControl(BPoint topleft,int numcolor):BControl(BRect(topleft.x,topleft.y,topleft.x+15,topleft.y+131),"",NULL,NULL,B_FOLLOW_NONE,B_WILL_DRAW|B_NAVIGABLE) {
	BMessage* m=new BMessage('colo');
	m->AddInt32("color",numcolor);
	SetMessage(m);
	this->numcolor=numcolor;
	SetViewColor(B_TRANSPARENT_32_BIT);
}

void DKColorControl::AttachedToWindow() {
	SetTarget(Window());
}

void DKColorControl::Draw(BRect) {
	DrawBitmap(ColorRamp[numcolor%3][internaldepth],BPoint(2,2));
	SetHighColor(white);
	int v=127-Value()/2;
	StrokeLine(BPoint(2,v+2),BPoint(13,v+2));
	if (IsFocus()) {
		SetHighColor(0,0,255);
		StrokeRect(BRect(0,0,15,131));
		StrokeRect(BRect(1,1,14,130));
	} else {
		SetHighColor(dark_gray);
		StrokeLine(BPoint(0,0),BPoint(14,0));
		StrokeLine(BPoint(0,1),BPoint(0,130));
		StrokeLine(BPoint(1,1),BPoint(13,1));
		StrokeLine(BPoint(1,2),BPoint(1,129));

		SetHighColor(light_gray);
		StrokeLine(BPoint(15,1),BPoint(15,131));
		StrokeLine(BPoint(1,131),BPoint(14,131));
		StrokeLine(BPoint(14,2),BPoint(14,130));
		StrokeLine(BPoint(2,130),BPoint(13,130));
		SetHighColor(medium_gray);
		StrokeLine(BPoint(15,0),BPoint(14,1));
		StrokeLine(BPoint(0,131),BPoint(1,130));
	}
}

void DKColorControl::MouseDown(BPoint) {
	resume_thread(tid=spawn_thread(mousethread,"Mouse Poller",B_DISPLAY_PRIORITY,this));
}

void DKColorControl::DetachedFromWindow() {
	if (tid!=0) {
		acquire_sem(mpsem);
		kill_thread(tid);
		release_sem(mpsem);
	}
}

void DKColorControl::SetValue(int32 v) {
	if (v!=Value()) {
		Invoke();
		BControl::SetValue(v);
		ticks=0;
	}
}

void DKColorControl::KeyDown(const char* c,int32 n) {
	switch(*c) {
		case B_UP_ARROW:
			if (Value()<254) SetValue(Value()+2);
			break;
		case B_DOWN_ARROW:
			if (Value()>1) SetValue(Value()-2);
			break;
		default:
			BControl::KeyDown(c,n);
	}
}

DKScreenView::DKScreenView():BView(BRect(327,10,361,38),"",B_FOLLOW_NONE,B_WILL_DRAW) {
}

void DKScreenView::AttachedToWindow() {
	BMenuItem* mitem;
	BMessage* mess;
	menu=new BPopUpMenu("",false,false);

	langitem.mitem9=mitem=new BMenuItem(text[9][language],new BMessage(B_ABOUT_REQUESTED));
	mitem->SetTarget(be_app);
	menu->AddItem(mitem);

	menu->AddSeparatorItem();

	BMenu* subm;
	langitem.submenu=subm=new BMenu(text[10][language]);
	for (int i=0;i<numlang;i++) {
		mess=new BMessage('lang');
		mess->AddInt32("lang",i);
		mitem=new BMenuItem(langname[i][language],mess);
		mitem->SetTarget(be_app);
		if (i==language) mitem->SetMarked(true);
		subm->AddItem(mitem);
	}
	langitem.mitem10=mitem=new BMenuItem(subm);
	menu->AddItem(mitem);

	langitem.mitem11=mitem=new BMenuItem(text[11][language],new BMessage('save'));
	mitem->SetTarget(be_app);
	menu->AddItem(mitem);

	langitem.mitem12=mitem=new BMenuItem(text[12][language],new BMessage(B_QUIT_REQUESTED));
	mitem->SetTarget(be_app);
	menu->AddItem(mitem);
	
	BScreen s;
	SetViewColor(s.DesktopColor());
}

void DKScreenView::Draw(BRect u) {
	if (!BRect(5,5,29,23).Contains(u)) {
		SetHighColor(medium_gray);
		FillRect(BRect(0,0,1,1));
		FillRect(BRect(33,0,34,1));
		FillRect(BRect(0,27,1,28));
		FillRect(BRect(33,27,34,28));
		SetHighColor(160,160,160);
		FillRect(BRect(1,1,33,4));
		FillRect(BRect(1,24,33,27));
		FillRect(BRect(1,1,4,27));
		FillRect(BRect(30,1,33,27));
		SetHighColor(black);
		StrokeLine(BPoint(2,0),BPoint(32,0));
		StrokeLine(BPoint(2,28),BPoint(32,28));
		StrokeLine(BPoint(0,2),BPoint(0,26));
		StrokeLine(BPoint(34,2),BPoint(34,26));
		StrokeLine(BPoint(1,1),BPoint(1,1));
		StrokeLine(BPoint(33,1),BPoint(33,1));
		StrokeLine(BPoint(1,27),BPoint(1,27));
		StrokeLine(BPoint(33,27),BPoint(33,27));
		StrokeLine(BPoint(5,4),BPoint(29,4));
		StrokeLine(BPoint(5,24),BPoint(29,24));
		StrokeLine(BPoint(4,5),BPoint(4,23));
		StrokeLine(BPoint(30,5),BPoint(30,23));
		SetHighColor(224,0,0);
		StrokeLine(BPoint(4,26),BPoint(5,26));
	}
}

void DKScreenView::MouseDown(BPoint p) {
	menu->Go(ConvertToScreen(p),true,true,ConvertToScreen(Bounds()),true);
}

DKPreview::DKPreview():BView(BRect(7,7,344,21),"",B_FOLLOW_NONE,B_WILL_DRAW) {
	SetViewColor(B_TRANSPARENT_32_BIT);
}

void DKPreview::Draw(BRect) {
	DrawBitmap(PreviewRamp[internaldepth],BPoint(0,0));
}

int main() {
	DKApplication _;
}

long dkthread(void*) {
	acquire_sem(ccsem);
	release_sem(ccsem);
	rgb_color c;
	while(true) {
		if ((ticks++&1023)==0) {
			time_t thetime;
			time(&thetime);
			struct tm *lcltime=localtime(&thetime);
			float phi=(lcltime->tm_hour*3600+lcltime->tm_min*60+lcltime->tm_sec)*(2*PI/86400);
			float p=(1-cos(phi));
			if (sin(phi)>=0) {
				if (p<1) {
					c.red=uint8(c6.red*p+c0.red*(1-p));
					c.green=uint8(c6.green*p+c0.green*(1-p));
					c.blue=uint8(c6.blue*p+c0.blue*(1-p));
				} else {
					c.red=uint8(c12.red*(p-1)+c6.red*(2-p));
					c.green=uint8(c12.green*(p-1)+c6.green*(2-p));
					c.blue=uint8(c12.blue*(p-1)+c6.blue*(2-p));
				}
			} else {
				if (p<1) {
					c.red=uint8(c18.red*p+c0.red*(1-p));
					c.green=uint8(c18.green*p+c0.green*(1-p));
					c.blue=uint8(c18.blue*p+c0.blue*(1-p));
				} else {
					c.red=uint8(c12.red*(p-1)+c18.red*(2-p));
					c.green=uint8(c12.green*(p-1)+c18.green*(2-p));
					c.blue=uint8(c12.blue*(p-1)+c18.blue*(2-p));
				}
			}
		}
		rgb_color cur;
		acquire_sem(ccsem);
		{
			BScreen s;
			cur=s.DesktopColor();
			if ((cur.red!=c.red)||(cur.green!=c.green)||(cur.blue!=c.blue)) {
				s.SetDesktopColor(c,false);
				scrview->Window()->Lock();
				scrview->SetViewColor(c);
				scrview->Invalidate(BRect(5,5,29,23));
				scrview->Sync();
				scrview->Window()->Unlock();
			}
		}
		release_sem(ccsem);
		snooze(80*1000);
	}
}

long mousethread(void* _p) {
	DKColorControl* cc=(DKColorControl*)_p;
	BPoint p;
	uint32 but;
	int v;
	do {
		acquire_sem(mpsem);
		cc->Window()->Lock();
		cc->GetMouse(&p,&but);
		if (but) {
			if (p.y<2) v=255;
			if ((p.y>=2)&&(p.y<130)) v=uint32(259-2*p.y);
			if (p.y>=130) v=0;
			if (v!=cc->Value()) {
				cc->SetValue(v);
			}
			cc->Window()->Unlock();
			release_sem(mpsem);
			snooze(15000);
		} else {
			release_sem(mpsem);
			cc->Window()->Unlock();
		}
	} while(but);
	cc->tid=0;
	return B_OK;
}

long bitmapthread(void*) {
	rgb_color l0,l6,l12,l18;
	uchar cvrt[216];
	{BScreen s;
		for (int b=0;b<6;b++) {
			for (int g=0;g<6;g++) {
				for (int r=0;r<6;r++) {
					cvrt[36*r+6*g+b]=s.IndexForColor(51*r,51*g,51*b);
				}
			}
		}
	}
	while (true) {
		acquire_sem(blsem);
		acquire_sem(bdsem);
		do {
			l0=c0;
			l6=c6;
			l12=c12;
			l18=c18;
			data_avail=false;
			thr_running=true;
			release_sem(bdsem);
			uchar* fb8=(uchar*)PreviewRamp[0]->Bits();
			uint32 ll8=PreviewRamp[0]->BytesPerRow();
			uchar* fb15=(uchar*)PreviewRamp[1]->Bits();
			uint32 ll15=PreviewRamp[1]->BytesPerRow();
			uchar* fb16=(uchar*)PreviewRamp[2]->Bits();
			uint32 ll16=PreviewRamp[2]->BytesPerRow();
			uchar* fb32=(uchar*)PreviewRamp[3]->Bits();
			uint32 ll32=PreviewRamp[3]->BytesPerRow();
			for (int i=0;i<=337;i++) {
				rgb_color c;
				float phi=2*PI*i/338;
				float p=(1-cos(phi));
				if (sin(phi)>=0) {
					if (p<1) {
						c.red=uint8(c6.red*p+c0.red*(1-p));
						c.green=uint8(c6.green*p+c0.green*(1-p));
						c.blue=uint8(c6.blue*p+c0.blue*(1-p));
					} else {
						c.red=uint8(c12.red*(p-1)+c6.red*(2-p));
						c.green=uint8(c12.green*(p-1)+c6.green*(2-p));
						c.blue=uint8(c12.blue*(p-1)+c6.blue*(2-p));
					}
				} else {
					if (p<1) {
						c.red=uint8(c18.red*p+c0.red*(1-p));
						c.green=uint8(c18.green*p+c0.green*(1-p));
						c.blue=uint8(c18.blue*p+c0.blue*(1-p));
					} else {
						c.red=uint8(c12.red*(p-1)+c18.red*(2-p));
						c.green=uint8(c12.green*(p-1)+c18.green*(2-p));
						c.blue=uint8(c12.blue*(p-1)+c18.blue*(2-p));
					}
				}
				for (int j=0;j<=14;j++) {
					uchar v8=36*(c.red/51)+6*(c.green/51)+(c.blue/51);
					if (float(rand())/RAND_MAX<(c.blue/51.)-(c.blue/51)) v8++;
					if (float(rand())/RAND_MAX<(c.green/51.)-(c.green/51)) v8+=6;
					if (float(rand())/RAND_MAX<(c.red/51.)-(c.red/51)) v8+=36;
					fb8[i+ll8*j]=cvrt[v8];
					uint16 v15=((c.red*31/255)<<10)+((c.green*31/255)<<5)+(c.blue*31/255);
					if (float(rand())/RAND_MAX<(c.blue*31./255)-(c.blue*31/255)) v15++;
					if (float(rand())/RAND_MAX<(c.green*31./255)-(c.green*31/255)) v15+=32;
					if (float(rand())/RAND_MAX<(c.red*31./255)-(c.red*31/255)) v15+=1024;
					*(uint16*)(fb15+i*2+j*ll15)=B_HOST_TO_LENDIAN_INT16(v15);
					uint16 v16=((c.red*31/255)<<11)+((c.green*63/255)<<5)+(c.blue*31/255);
					if (float(rand())/RAND_MAX<(c.blue*31./255)-(c.blue*31/255)) v16++;
					if (float(rand())/RAND_MAX<(c.green*63./255)-(c.green*63/255)) v16+=32;
					if (float(rand())/RAND_MAX<(c.red*31./255)-(c.red*31/255)) v16+=2048;
					*(uint16*)(fb16+i*2+j*ll16)=B_HOST_TO_LENDIAN_INT16(v16);
					fb32[i*4+j*ll32]=c.blue;
					fb32[i*4+j*ll32+1]=c.green;
					fb32[i*4+j*ll32+2]=c.red;
				}
			}
			acquire_sem(bpsem);
			preview->Window()->Lock();
			preview->Invalidate();
			preview->Sync();
			preview->Window()->Unlock();
			release_sem(bpsem);
			acquire_sem(bdsem);
		} while (data_avail);
		thr_running=false;
		release_sem(bdsem);
	}	
}
