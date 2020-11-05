//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <math.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

int* BHT;
uint32_t GHR;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  uint32_t i;
  int bht_size = pow(2, ghistoryBits);

  BHT = (int*)malloc(bht_size*sizeof(int));

  for(i=0;i<bht_size;i++)
  {
    BHT[i] = WN;
  }

  GHR = 0;
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken

uint8_t make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //
  uint8_t prediction;
  uint32_t index;
  uint32_t bht_size = pow(2, ghistoryBits);
  uint32_t pc_low = pc % bht_size;
  index = pc_low ^ GHR;

  if( BHT[index]==WT || BHT[index]==ST)
  {
    prediction = TAKEN;
  }
  else
  {
    prediction = NOTTAKEN;
  }
  
  // Make a prediction based on the bpType
  switch (bpType)
  {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return prediction;
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  uint32_t index;
  uint32_t bht_size = pow(2, ghistoryBits);
  uint32_t pc_low = pc % bht_size;
  index = pc_low ^ GHR;

  if(outcome==TAKEN)
  {
    if(BHT[index]!=ST)
    {
      BHT[index] = BHT[index] + 1;
    }
  }
  else
  {
    if(BHT[index]!=SN);
    {
      BHT[index] = BHT[index] - 1;
    }
  }

  GHR = ((GHR<<1) % bht_size) + outcome;
}
