#ifndef CPUMETER_H
#define CPUMETER_H

//
//  Copyright (c) 1994, 1995, 2004, 2006 by Mike Romberg ( mike.romberg@noaa.gov )
//
//  This file may be distributed under terms of the GPL
//

#include "fieldmetergraph.h"
#include "osview.h"


class CPUMeter : public FieldMeterGraph {
public:
  CPUMeter(XOSView *parent, const char *cpuID = "cpu");
  ~CPUMeter(void);

  const char *name(void) const { return "CPUMeter"; }
  void checkevent(void);

  void checkResources(void);

  static int countCPUs(void);
  static const char *cpuStr(int num);
  static int getkernelversion(void);
protected:
  int _lineNum;
  unsigned long long cputime_[2][10];
  int cpuindex_;
  int kernel_;
  int statfields_;

  void getcputime(void);
  int findLine(const char *cpuID);
  const char *toUpper(const char *str);
private:
};

#endif // CPUMETER_H
