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

int *Global_BHT;
uint32_t *PHT;
int *Local_BHT;
uint32_t GHR;
int *CHT;

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
  int global_bht_size = pow(2, ghistoryBits);
  int pht_size = pow(2, pcIndexBits);
  int local_bht_size = pow(2, lhistoryBits);
  int cht_size = pow(2, ghistoryBits);

  Global_BHT = (int*)malloc(global_bht_size*sizeof(int));
  PHT = (uint32_t*)malloc(pht_size*sizeof(uint32_t));
  Local_BHT = (int*)malloc(local_bht_size*sizeof(int));
  CHT = (int*)malloc(cht_size*sizeof(int));

  for(i=0;i<global_bht_size;i++)
  {
    Global_BHT[i] = WN;
  }

  for(i=0;i<pht_size;i++)
  {
    PHT[i] = NOTTAKEN;
  }

  for(i=0;i<local_bht_size;i++)
  {
    Local_BHT[i] = WN;
  }

  for(i=0;i<cht_size;i++)
  {
    CHT[i] = WG;
  }

  GHR = 0;
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken

uint8_t gshare_predictor(uint32_t pc)               //Gshare prediction scheme implementation
{
  uint8_t prediction;
  uint32_t global_bht_size = pow(2, ghistoryBits);
  uint32_t pc_low = pc % global_bht_size;
  uint32_t index = pc_low ^ GHR;

  prediction = (Global_BHT[index]>=WT) ? TAKEN : NOTTAKEN;

  return prediction;
}

uint8_t tournament_predictor(uint32_t pc)           //Tournament prediction scheme implementation
{
  uint8_t local_prediction, global_prediction, prediction;
  uint32_t pht_size = pow(2, pcIndexBits);
  uint32_t pc_low = pc % pht_size;
  uint32_t local_index = pc_low;

  local_prediction = (Local_BHT[PHT[local_index]]>=WT) ? TAKEN : NOTTAKEN;
  global_prediction = (Global_BHT[GHR]>=WT) ? TAKEN : NOTTAKEN;

  prediction = (CHT[GHR]>=SL) ? local_prediction : global_prediction;

  return prediction;
}

uint8_t make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //
  
  // Make a prediction based on the bpType
  switch (bpType)
  {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gshare_predictor(pc);
    case TOURNAMENT:
      return tournament_predictor(pc);
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

void train_gshare_predictor(uint32_t pc, uint8_t outcome)                 //Training Gshare Predictor
{
  uint32_t global_bht_size = pow(2, ghistoryBits);
  uint32_t pc_low = pc % global_bht_size;
  uint32_t index = pc_low ^ GHR;

  if(outcome==TAKEN)
  {
    if(Global_BHT[index]!=ST)
    {
      Global_BHT[index] = Global_BHT[index] + 1;
    }
  }
  else
  {
    if(Global_BHT[index]!=SN);
    {
      Global_BHT[index] = Global_BHT[index] - 1;
    }
  }

  GHR = ((GHR<<1) % global_bht_size) + outcome;
}

void train_tournament_predictor(uint32_t pc, uint8_t outcome)             //Training Tournament Predictor
{
  uint32_t pht_size = pow(2, pcIndexBits);
  uint32_t pc_low = pc % pht_size;
  uint32_t local_index = pc_low;
  uint32_t local_bht_size = pow(2, lhistoryBits);
  uint32_t global_bht_size = pow(2, ghistoryBits);

  uint8_t local_prediction = (Local_BHT[PHT[local_index]]>=WT) ? TAKEN : NOTTAKEN;
  uint8_t global_prediction = (Global_BHT[GHR]>=WT) ? TAKEN : NOTTAKEN;

  if(local_prediction!=global_prediction)
  {
    if((local_prediction==outcome) && (CHT[GHR]!=SL))
    {
      CHT[GHR] = CHT[GHR] + 1;
    }
    else if((global_prediction==outcome) && (CHT[GHR]!=SG))
    {
      CHT[GHR] = CHT[GHR] - 1;
    }
  }

  if(outcome==TAKEN)
  {
    if(Local_BHT[PHT[local_index]]!=ST)
    {
      Local_BHT[PHT[local_index]] = Local_BHT[PHT[local_index]] + 1;
    }
  }
  else
  {
    if(Local_BHT[PHT[local_index]]!=SN);
    {
      Local_BHT[PHT[local_index]] = Local_BHT[PHT[local_index]] - 1;
    }
  }

  PHT[local_index] = ((PHT[local_index]<<1) % local_bht_size) + outcome;

  if(outcome==TAKEN)
  {
    if(Global_BHT[GHR]!=ST)
    {
      Global_BHT[GHR] = Global_BHT[GHR] + 1;
    }
  }
  else
  {
    if(Global_BHT[GHR]!=SN);
    {
      Global_BHT[GHR] = Global_BHT[GHR] - 1;
    }
  }

  GHR = ((GHR<<1) % global_bht_size) + outcome;
}

void train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  switch (bpType)
  {
    case GSHARE:
      train_gshare_predictor(pc, outcome);
    case TOURNAMENT:
      train_tournament_predictor(pc, outcome);
    case CUSTOM:
    default:
      break;
  }
}
