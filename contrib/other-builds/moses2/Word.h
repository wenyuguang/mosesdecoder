/*
 * Word.h
 *
 *  Created on: 23 Oct 2015
 *      Author: hieu
 */

#pragma once

#include "Factor.h"
#include "TypeDef.h"

class Word {
public:
	Word();
	virtual ~Word();

protected:
  Factor *m_factors[NUM_FACTOR];
};

