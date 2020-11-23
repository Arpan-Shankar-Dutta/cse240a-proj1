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

//Gshare structures
uint8_t *Global_BHT_G;
uint32_t GHR_G;

//tournament structures
uint8_t *Global_BHT;
uint32_t *PHT;
uint8_t *Local_BHT;
uint32_t GHR;
uint8_t *CHT;

//Custom structures
uint8_t *Global_BHT_C;                //2-bit predictors
uint32_t GHR_C;                       //Global history register


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

  uint32_t global_bht_size_G = pow(2, ghistoryBits);

  uint32_t global_bht_size = pow(2, ghistoryBits);
  uint32_t pht_size = pow(2, pcIndexBits);
  uint32_t local_bht_size = pow(2, lhistoryBits);
  uint32_t cht_size = pow(2, ghistoryBits);

  uint32_t global_bht_size_C = pow(2, 13);                        //2-bit predictor size calculation ( Total size = 2-bit * 2^13 entries = 16KB )

  Global_BHT_G = malloc(global_bht_size_G*sizeof(uint8_t));

  Global_BHT = malloc(global_bht_size*sizeof(uint8_t));
  PHT = malloc(pht_size*sizeof(uint32_t));
  Local_BHT = malloc(local_bht_size*sizeof(uint8_t));
  CHT = malloc(cht_size*sizeof(uint8_t));

  Global_BHT_C = malloc(global_bht_size_C*sizeof(uint8_t));       //2-bit predictor memory allocation

  for(i=0;i<global_bht_size_G;i++)
  {
    Global_BHT_G[i] = WN;
  }

  GHR_G = NOTTAKEN;

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

  GHR = NOTTAKEN;

  for(i=0;i<global_bht_size_C;i++)                        //initializing Custom predictor data structure to Weakly Nottaken
  {
    Global_BHT_C[i] = WN;
  }

  GHR_C = NOTTAKEN;                                       //initializing Custom predictor Global history to Nottaken
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken

uint8_t gshare_predictor(uint32_t pc)               //Gshare prediction scheme implementation
{
  uint8_t prediction;
  uint32_t global_bht_size_G = pow(2, ghistoryBits) - 1;
  uint32_t pc_low = pc & global_bht_size_G;
  uint32_t index = pc_low ^ GHR_G;

  prediction = (Global_BHT_G[index]>=WT) ? TAKEN : NOTTAKEN;

  return prediction;
}

uint8_t tournament_predictor(uint32_t pc)           //Tournament prediction scheme implementation
{
  uint8_t local_prediction, global_prediction, prediction;
  uint32_t pht_size = pow(2, pcIndexBits) - 1;
  uint32_t pc_low = pc & pht_size;
  uint32_t local_index = pc_low;

  local_prediction = (Local_BHT[PHT[local_index]]>=WT) ? TAKEN : NOTTAKEN;
  global_prediction = (Global_BHT[GHR]>=WT) ? TAKEN : NOTTAKEN;

  prediction = (CHT[GHR]>=WL) ? local_prediction : global_prediction;

  return prediction;
}

uint8_t custom_predictor(uint32_t pc)
{
  uint8_t prediction_pc, prediction_ghr, prediction_gshare, prediction;
  uint32_t global_bht_size = pow(2, 13) - 1;        //Mask for extracting lower 13 bits of PC
  uint32_t pc_low = pc & global_bht_size;           //Calculating lower 13 bits of PC

  uint32_t index0 = pc_low ^ GHR_C;                 //Xor of PC and GHR yields index that varies more and aliases less(Random indexing based on a hash function)
  uint32_t index1 = pc_low;                         //Using PC directly to index predictors allows for storing prediction for a specific given PC(local prediction)
  uint32_t index2 = GHR_C;                          //Using GHR directly to index predictors allows for storing prediction for a Global pattern(Global prediction)

  //It is unlikely that all the entires of the 2-bit predictor table will be accessed. In fact many of the entires
  //are not accesed and hence the space alloted to them is wasted. Therefore having 3 indices to the same predictor
  //table allows better usage of the available resources by predicting based on different logic. Also using this instead
  //of using multiple ways allows for one large table (can hold more values for a fixed size of predictor) and minimal overhead.

  prediction_gshare = (Global_BHT_C[index0]>=WT) ? TAKEN : NOTTAKEN;      //predict taken if counter>=2 else nottaken
  prediction_pc = (Global_BHT_C[index1]>=WT) ? TAKEN : NOTTAKEN;
  prediction_ghr = (Global_BHT_C[index2]>=WT) ? TAKEN : NOTTAKEN;

  //combining the 3 predictions from the 3 indices using majority function
  prediction = (prediction_gshare & prediction_pc) | (prediction_ghr & prediction_pc) | (prediction_ghr & prediction_gshare);

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
      break;
    case GSHARE:
      return gshare_predictor(pc);
      break;
    case TOURNAMENT:
      return tournament_predictor(pc);
      break;
    case CUSTOM:
      return custom_predictor(pc);
      break;
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
  uint32_t global_bht_size_G = pow(2, ghistoryBits) - 1;
  uint32_t pc_low = pc & global_bht_size_G;
  uint32_t index = pc_low ^ GHR_G;

  if(outcome==TAKEN && Global_BHT_G[index]<ST)
  {
    Global_BHT_G[index]++;
  }
  
  if(outcome==NOTTAKEN && Global_BHT_G[index]>SN)
  {
    Global_BHT_G[index]--;
  }

  GHR_G = ((GHR_G<<1) & global_bht_size_G) + outcome;
}

void train_tournament_predictor(uint32_t pc, uint8_t outcome)             //Training Tournament Predictor
{
  uint32_t pht_size = pow(2, pcIndexBits) - 1;
  uint32_t pc_low = pc & pht_size;
  uint32_t local_index = pc_low;
  uint32_t local_bht_size = pow(2, lhistoryBits) - 1;
  uint32_t global_bht_size = pow(2, ghistoryBits) - 1;

  uint8_t local_prediction = (Local_BHT[PHT[local_index]]>=WT) ? TAKEN : NOTTAKEN;
  uint8_t global_prediction = (Global_BHT[GHR]>=WT) ? TAKEN : NOTTAKEN;

  if(local_prediction!=global_prediction && local_prediction==outcome && CHT[GHR]<SL)
  {
    CHT[GHR]++;
  }
  
  if(local_prediction!=global_prediction && global_prediction==outcome && CHT[GHR]>SG)
  {
    CHT[GHR]--;
  }
  
  if(outcome==TAKEN && Local_BHT[PHT[local_index]]<ST)
  {
    Local_BHT[PHT[local_index]]++;
  }

  if(outcome==NOTTAKEN && Local_BHT[PHT[local_index]]>SN)
  {
    Local_BHT[PHT[local_index]]--;
  }

  if(outcome==TAKEN && Global_BHT[GHR]<ST)
  {
    Global_BHT[GHR]++;
  }

  if(outcome==NOTTAKEN && Global_BHT[GHR]>SN)
  {
    Global_BHT[GHR]--;
  }

  PHT[local_index] = ((PHT[local_index]<<1) & local_bht_size) + outcome;
  GHR = ((GHR<<1) & global_bht_size) + outcome;
}

void train_custom_predictor(uint32_t pc, uint8_t outcome)
{
  uint32_t global_bht_size = pow(2, 13) - 1;          //calculating index and size similar to custom prediction function
  uint32_t pc_low = pc & global_bht_size;

  uint32_t index0 = pc_low ^ GHR_C;
  uint32_t index1 = pc_low;
  uint32_t index2 = GHR_C;

  if(outcome==TAKEN && Global_BHT_C[index0]<ST)       //incrementing or decrementing the 2-bit saturarting counters at all the 3 indices based on the outcome
  {
    Global_BHT_C[index0]++;
  }
  if(outcome==NOTTAKEN && Global_BHT_C[index0]>SN)
  {
    Global_BHT_C[index0]--;
  }
  
  if(outcome==TAKEN && Global_BHT_C[index1]<ST)
  {
    Global_BHT_C[index1]++;
  }
  if(outcome==NOTTAKEN && Global_BHT_C[index1]>SN)
  {
    Global_BHT_C[index1]--;
  }

  if(outcome==TAKEN && Global_BHT_C[index2]<ST)
  {
    Global_BHT_C[index2]++;
  }
  if(outcome==NOTTAKEN && Global_BHT_C[index2]>SN)
  {
    Global_BHT_C[index2]--;
  }

  //Size of GHR is 13 bit < 256-bit
  GHR_C = ((GHR_C<<1) & global_bht_size) + outcome;   //Updating Global history based on outcome by left shifting GHR and placing the new outcome at LSB
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
      break;
    case TOURNAMENT:
      train_tournament_predictor(pc, outcome);
      break;
    case CUSTOM:
      train_custom_predictor(pc, outcome);
      break;
    default:
      break;
  }
}
