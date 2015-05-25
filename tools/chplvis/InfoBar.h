/*
 * Copyright 2015 Cray Inc.
 * Other additional copyright holders may be indicated within.
 *
 * The entirety of this work is licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INFOBAR_H
#define INFOBAR_H

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <string.h>

class InfoBar : public Fl_Box {

 private:
  int maxTasks;
  int maxComms;
  long maxSize;
  double maxCpu;
  bool showtasks;
  bool showcomms;

  char *fileName;
  char *tagName;

 public:

  InfoBar (int x, int y, int w, int h/*, const char *label = 0*/)
#ifdef __APPLE__    
    : Fl_Box(x,0,w,h+20) {
#else
    : Fl_Box(x,y,w,h) {
#endif
      printf ("InfoBar constructor...\n");
    maxTasks = 0;
    maxComms = 0;
    maxSize = 0;
    maxCpu = 0;
    showtasks = true;
    showcomms = true;
    fileName = NULL;
    tagName = NULL;
   };

  void draw(void);

  void setMaxes(int tasks, int comms, long size, double cpu) {
    maxTasks = tasks;
    maxComms = comms;
    maxSize = size;
    maxCpu = cpu;
  }

  void setFileName(const char *name);
  void setTagName(const char *name);

  void showTasks() { showtasks = true; }
  void showCpu() { showtasks = false; }
  void showComms() { showcomms = true; }
  void showSize() { showcomms = false; }

};


#endif

