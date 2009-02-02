/***********************************************************************
Moses - factored phrase-based language decoder
Copyright (C) 2009 University of Edinburgh

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
***********************************************************************/

#include "TranslationDelta.h"

using namespace std;

namespace Moses {

/**
  Compute the change in language model score by adding this target phrase
  into the hypothesis at the given target position.
  **/
void  TranslationDelta::addLanguageModelScore(const vector<Word>& targetWords, const Phrase& targetPhrase, const WordsRange& targetSegment) {
  const LMList& languageModels = StaticData::Instance().GetAllLM();
  for (LMList::const_iterator i = languageModels.begin(); i != languageModels.end(); ++i) {
    LanguageModel* lm = *i;
    size_t order = lm->GetNGramOrder();
    vector<const Word*> lmcontext(targetPhrase.GetSize() + 2*(order-1));
    
    int start = targetSegment.GetStartPos() - (order-1);
    
    //fill in the pre-context
    size_t contextPos = 0;

    for (size_t i = 0; i < order-1; ++i) {
      if (start+(int)i < 0) {
        lmcontext[contextPos++] = &(lm->GetSentenceStartArray());
      } else {
        lmcontext[contextPos++] = &(targetWords[i+start]);
      }
    }
    
    //fill in the target phrase
    for (size_t i = 0; i < targetPhrase.GetSize(); ++i) {
      lmcontext[contextPos++] = &(targetPhrase.GetWord(i));
    }
    
    //fill in the postcontext
    size_t eoscount = 0;
    for (size_t i = 0; i < order-1; ++i) {
      size_t targetPos = i + targetSegment.GetEndPos() + 1;
      if (targetPos >= targetWords.size()) {
        lmcontext[contextPos++] = &(lm->GetSentenceEndArray());
        ++eoscount;
      } else {
        lmcontext[contextPos++] = &(targetWords[targetPos]);
      }
    }
    
    //debug
    IFVERBOSE(3) {
      VERBOSE(3,"Segment: " << targetSegment << " phrase: " << targetPhrase << endl);
      VERBOSE(3,"LM context ");
      for (size_t j = 0;  j < lmcontext.size(); ++j) {
        if (j == order-1) {
          VERBOSE(3, "[ ");
        }
        if (j == (targetPhrase.GetSize() + (order-1))) {
          VERBOSE(3,"] ");
        }
        VERBOSE(3,*(lmcontext[j]) << " ");
        
      }
      VERBOSE(3,endl);
    }
    
    //score lm
    double lmscore = 0;
    vector<const Word*> ngram;
    //remember to only include max of 1 eos marker
    size_t maxend = min(lmcontext.size(), lmcontext.size() - (eoscount-1));
    for (size_t ngramstart = 0; ngramstart < lmcontext.size() - (order -1); ++ngramstart) {
      ngram.clear();
      for (size_t j = ngramstart; j < ngramstart+order && j < maxend; ++j) {
        ngram.push_back(lmcontext[j]);
      }
      lmscore += lm->GetValue(ngram);
    }
    VERBOSE(2,"Language model score: " << lmscore << endl); 
    m_scores.Assign(lm,lmscore);
  }
}


TranslationUpdateDelta::TranslationUpdateDelta(const vector<Word>& targetWords,  const TranslationOption* option , 
                                               const WordsRange& targetSegment) :
    m_option(option)  {
  //translation scores
  m_scores.PlusEquals(m_option->GetScoreBreakdown());
        
  //don't worry about reordering because they don't change
        
  //word penalty
  float penalty = -((int)targetSegment.GetNumWordsCovered());
  m_scores.Assign(StaticData::Instance().GetWordPenaltyProducer(),penalty);
        
  IFVERBOSE(2) {
    //translation scores are already there
    const vector<PhraseDictionary*>& translationModels = StaticData::Instance().GetPhraseDictionaries();
    //cout << "Got " << translationModels.size() << " dictionary(s)" << endl;
    for (vector<PhraseDictionary*>::const_iterator i = translationModels.begin(); i != translationModels.end(); ++i) {
      vector<float> translationScores = m_scores.GetScoresForProducer(*i);
      VERBOSE(2,"Translation scores: ");
      
      for (size_t j = 0; j < translationScores.size(); ++j) {
        VERBOSE(2,translationScores[j] << " ");
      }
      VERBOSE(2,endl);
              
      
    }
  }
        
  addLanguageModelScore(targetWords, m_option->GetTargetPhrase(), targetSegment);
        
  //weight the scores
  const vector<float> & weights = StaticData::Instance().GetAllWeights();
        //cout << " weights: ";
        //copy(weights.begin(), weights.end(), ostream_iterator<float>(cout," "));
        //cout << endl;
  m_score = m_scores.InnerProduct(weights);
  VERBOSE(2, "Scores " << m_scores << endl);
  VERBOSE(2,"Total score is  " << m_score << endl);  
}

void TranslationUpdateDelta::apply(Sample& sample, const TranslationDelta& noChangeDelta) {
  m_scores.MinusEquals(noChangeDelta.getScores());
  sample.ChangeTarget(*m_option,m_scores);
}


MergeDelta::MergeDelta(const vector<Word>& targetWords, const TranslationOption* option, const WordsRange& targetSegment) {
  //TODO
}

void MergeDelta::apply(Sample& sample, const TranslationDelta& noChangeDelta) {
  //TODO
}

PairedTranslationUpdateDelta::PairedTranslationUpdateDelta(const vector<Word>& targetWords,
   const TranslationOption* leftOption, const TranslationOption* rightOption, 
   const WordsRange& leftTargetSegment, const WordsRange& rightTargetSegment) {
     //TODO
}

void PairedTranslationUpdateDelta::apply(Sample& sample, const TranslationDelta& noChangeDelta) {
  //TODO
}

SplitDelta::SplitDelta(const vector<Word>& targetWords,
    const TranslationOption* leftOption, const TranslationOption* rightOption, const WordsRange& targetSegment) {
 //TODO
}

void SplitDelta::apply(Sample& sample, const TranslationDelta& noChangeDelta) {
  //TODO
}

}//namespace
