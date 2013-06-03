/***************************************************************************
 *                           LIFTimeDrivenModel_1_4_GPU2.h                 *
 *                           -------------------                           *
 * copyright            : (C) 2012 by Francisco Naveros                    *
 * email                : fnaveros@atc.ugr.es                              *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LIFTIMEDRIVENMODEL_1_4_GPU2_H_
#define LIFTIMEDRIVENMODEL_1_4_GPU2_H_

/*!
 * \file LIFTimeDrivenModel_GPU.h
 *
 * \author Francisco Naveros
 * \date November 2012
 *
 * This file declares a class which abstracts a Leaky Integrate-And-Fire neuron model with one 
 * differential equation and four time dependent equations (conductances). This model is
 * implemented in GPU.
 */

#include "./TimeDrivenNeuronModel_GPU2.h"
#include "../../include/integration_method/IntegrationMethod_GPU2.h"
#include "../../include/integration_method/LoadIntegrationMethod_GPU2.h"

//Library for CUDA
#include <cutil_inline.h>

/*!
 * \class LIFTimeDrivenModel_GPU
 *
 * \brief Leaky Integrate-And-Fire Time-Driven neuron model
 *
 * This class abstracts the behavior of a neuron in a time-driven spiking neural network.
 * It includes internal model functions which define the behavior of the model
 * (initialization, update of the state, synapses effect, next firing prediction...).
 * This is only a virtual function (an interface) which defines the functions of the
 * inherited classes.
 *
 * \author Francisco Naveros
 * \date January 2012
 */

class LIFTimeDrivenModel_1_4_GPU2 : public TimeDrivenNeuronModel_GPU2 {
	public:
		/*!
		 * \brief Excitatory reversal potential
		 */
		float eexc;

		/*!
		 * \brief Inhibitory reversal potential
		 */
		float einh;

		/*!
		 * \brief Resting potential
		 */
		float erest;

		/*!
		 * \brief Firing threshold
		 */
		float vthr;

		/*!
		 * \brief Membrane capacitance
		 */
		float cm;

		/*!
		 * \brief AMPA receptor time constant
		 */
		float tampa;

		/*!
		 * \brief NMDA receptor time constant
		 */
		float tnmda;
		
		/*!
		 * \brief GABA receptor time constant
		 */
		float tinh;

		/*!
		 * \brief Gap Junction time constant
		 */
		float tgj;

		/*!
		 * \brief Refractory period
		 */
		float tref;

		/*!
		 * \brief Resting conductance
		 */
		float grest;

		/*!
		 * \brief Gap junction factor
		 */
		float fgj;

		/*!
		 * \brief Number of state variables for each cell.
		*/
		static const int N_NeuronStateVariables=5;

		/*!
		 * \brief Number of state variables witch are calculate with a differential equation for each cell.
		*/
		static const int N_DifferentialNeuronState=1;

		/*!
		 * \brief Number of state variables witch are calculate with a time dependent equation for each cell.
		*/
		static const int N_TimeDependentNeuronState=4;


		/*!
		 * \brief constructor with parameters.
		 *
		 * It generates a new neuron model object.
		 *
		 * \param Eexc eexc.
		 * \param Einh einh.
		 * \param Erest erest.
		 * \param Vthr vthr.
		 * \param Cm cm.
		 * \param Tampa tampa.
		 * \param Tnmda tnmda.
		 * \param Tinh tinh.
		 * \param Tgj tgj.
		 * \param Tref tref.
		 * \param Grest grest.
		 * \param Fgj fgj.
		 * \param integrationName integration method type.
		 * \param N_neurons number of neurons.
		 * \param Total_N_thread total number of CUDA thread.
		 * \param Buffer_GPU Gpu auxiliar memory.
		 *
		 */
		__device__ LIFTimeDrivenModel_1_4_GPU2(float Eexc,float Einh,float Erest,float Vthr,float Cm,float Tampa,
			float Tnmda,float Tinh,float Tgj,float Tref,float Grest,float Fgj, char const* integrationName, int N_neurons, int Total_N_thread, void ** Buffer_GPU):TimeDrivenNeuronModel_GPU2(),
			eexc(Eexc),einh(Einh),erest(Erest),vthr(Vthr),cm(Cm),tampa(Tampa),tnmda(Tnmda),tinh(Tinh),tgj(Tgj),
			tref(Tref),grest(Grest),fgj(Fgj){
			integrationMethod_GPU2=LoadIntegrationMethod_GPU2::loadIntegrationMethod_GPU2(integrationName, N_NeuronStateVariables, N_DifferentialNeuronState, N_TimeDependentNeuronState, Total_N_thread, Buffer_GPU);
		}

		/*!
		 * \brief Class destructor.
		 *
		 * It destroys an object of this class.
		 */
		__device__ ~LIFTimeDrivenModel_1_4_GPU2(){
		}


		/*!
		 * \brief Update the neuron state variables.
		 *
		 * It updates the neuron state variables.
		 *
		 * \param index The cell index inside the StateGPU. 
		 * \param AuxStateGPU Auxiliary incremental conductance vector.
		 * \param StateGPU Neural state variables.
		 * \param LastUpdateGPU Last update time
		 * \param LastSpikeTimeGPU Last spike time
		 * \param InternalSpikeGPU In this vector is stored if a neuron must generate an output spike.
		 * \param SizeStates Number of neurons
		 * \param CurrentTime Current time.
		 *
		 * \return True if an output spike have been fired. False in other case.
		 */
		__device__ void UpdateState(int index, float * AuxStateGPU, float * StateGPU, double * LastUpdateGPU, double * LastSpikeTimeGPU, bool * InternalSpikeGPU, int SizeStates, double CurrentTime)
		{
			double elapsed_time =CurrentTime - LastUpdateGPU[index];
			
			LastSpikeTimeGPU[index]+=elapsed_time;
			double last_spike=LastSpikeTimeGPU[index];


			StateGPU[1*SizeStates + index]+=AuxStateGPU[0*SizeStates + index];
			StateGPU[2*SizeStates + index]+=AuxStateGPU[1*SizeStates + index];
			StateGPU[3*SizeStates + index]+=AuxStateGPU[2*SizeStates + index];
			StateGPU[4*SizeStates + index]+=AuxStateGPU[3*SizeStates + index];

			bool spike = false;

			if (last_spike > this->tref) {
				integrationMethod_GPU2->NextDifferentialEcuationValue(index, SizeStates, this, StateGPU, elapsed_time);
				float vm_cou = StateGPU[0*SizeStates + index] + this->fgj * StateGPU[4*SizeStates + index];
				if (vm_cou > this->vthr){
					LastSpikeTimeGPU[index]=0;
					spike = true;
					StateGPU[0*SizeStates + index] = this->erest;
					this->integrationMethod_GPU2->resetState(index);
				}
			}else{
				EvaluateTimeDependentEcuation(index, SizeStates, StateGPU, elapsed_time);
			}

			InternalSpikeGPU[index]=spike;

		

			LastUpdateGPU[index]=CurrentTime;
		} 


		/*!
		 * \brief It evaluates the differential equation in NeuronState and it stores the results in AuxNeuronState.
		 *
		 * It evaluates the differential equation in NeuronState and it stores the results in AuxNeuronState.
		 *
		 * \param index index inside the NeuronState vector.
		 * \param SizeStates number of element in NeuronState vector.
		 * \param NeuronState value of the neuron state variables where differential equations are evaluated.
		 * \param AuxNeuronState results of the differential equations evaluation.
		 */
		__device__ void EvaluateDifferentialEcuation(int index, int SizeStates, float * NeuronState, float * AuxNeuronState){
			float iampa = NeuronState[1*SizeStates + index]*(this->eexc-NeuronState[0*SizeStates + index]);
			//float gnmdainf = 1.0/(1.0 + exp(-62.0*NeuronState[0])*1.2/3.57);
			float gnmdainf = 1.0/(1.0 + exp(-62.0*NeuronState[0*SizeStates + index])*0.336134453);
			float inmda = NeuronState[2*SizeStates + index]*gnmdainf*(this->eexc-NeuronState[0*SizeStates + index]);
			float iinh = NeuronState[3*SizeStates + index]*(this->einh-NeuronState[0*SizeStates + index]);
			AuxNeuronState[0*gridDim.x * blockDim.x + blockDim.x*blockIdx.x + threadIdx.x]=(iampa + inmda + iinh + this->grest* (this->erest-NeuronState[0*SizeStates + index]))*1.e-9/this->cm;
		}


		/*!
		 * \brief It evaluates the time depedendent ecuation in NeuronState for elapsed_time and it stores the results in NeuronState.
		 *
		 * It evaluates the time depedendent ecuation in NeuronState for elapsed_time and it stores the results in NeuronState.
		 *
		 * \param index index inside the NeuronState vector.
		 * \param SizeStates number of element in NeuronState vector.
		 * \param NeuronState value of the neuron state variables where time dependent equations are evaluated.
		 * \param elapsed_time integration time step.
		 */
		__device__ void EvaluateTimeDependentEcuation(int index, int SizeStates, float * NeuronState, double elapsed_time){
			NeuronState[1*SizeStates + index]*= exp(-(elapsed_time/this->tampa));
			NeuronState[2*SizeStates + index]*= exp(-(elapsed_time/this->tnmda));
			NeuronState[3*SizeStates + index]*= exp(-(elapsed_time/this->tinh));
			NeuronState[4*SizeStates + index]*= exp(-(elapsed_time/this->tgj));
		}



};


#endif /* LIFTIMEDRIVENMODEL_1_4_GPU2_H_ */
