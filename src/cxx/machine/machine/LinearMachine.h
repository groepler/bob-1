/**
 * @author Andre Anjos <andre.anjos@idiap.ch>
 * @date Mon 30 May 2011 12:27:03 CEST
 *
 * A machine that implements the liner projection of input to the output using
 * weights, biases and sums:
 *
 * output = sum (weights * inputs) + bias
 *
 * It is possible to setup the machine to previously normalize the input taking
 * into consideration some input bias and division factor. It is also possible
 * to set it up to have an activation function.
 *
 * A linear classifier. See C. M. Bishop, "Pattern Recognition and Machine
 * Learning", chapter 4
 */

#ifndef TORCH_MACHINE_LINEARMACHINE_H
#define TORCH_MACHINE_LINEARMACHINE_H

#include <blitz/array.h>
#include "database/HDF5File.h"

namespace Torch { namespace machine {

  /**
   * A linear classifier. See C. M. Bishop, "Pattern Recognition and Machine
   * Learning", chapter 4 for more details.
   */
  class LinearMachine {

    public: //api

      typedef enum Activation {
        LINEAR = 0, //Linear: y = x [this is the default]
        TANH = 1, //Hyperbolic tangent: y = (e^x - e^(-x))/(e^x + e^(-x))
        LOG = 2 //Logistic function: y = 1/(1 + e^(-x))
      } Activation;

      /**
       * Constructor, builds a new Linear machine. Weights and biases are
       * not initialized.
       *
       * @param input Size of input vector
       * @param output Size of output vector
       */
      LinearMachine (size_t input, size_t output);

      /**
       * Builds a new machine with a set of weights and biases. We will check
       * that the number of inputs (first dimension of weights) matches the
       * number of biases and will raise an exception if that is not the case.
       */
      LinearMachine(const blitz::Array<double,2>& weight, const
          blitz::Array<double,1>& bias);

      /**
       * Copies another machine
       */
      LinearMachine (const LinearMachine& other);

      /**
       * Starts a new LinearMachine from an existing Configuration object.
       */
      LinearMachine (Torch::database::HDF5File& config);

      /**
       * Just to virtualise the destructor
       */
      virtual ~LinearMachine();

      /**
       * Assigns from a different machine
       */
      LinearMachine& operator= (const LinearMachine& other);

      /**
       * Loads data from an existing configuration object. Resets the current
       * state.
       */
      void load (Torch::database::HDF5File& config);

      /**
       * Saves an existing machine to a Configuration object.
       */
      void save (Torch::database::HDF5File& config) const;

      /**
       * Forwards data through the network, outputs the values of each linear
       * component the input signal is decomposed at.
       *
       * The input and output are checked for compatibility each time the
       * forward method is applied.
       */
      void forward (const blitz::Array<double,1>& input,
          blitz::Array<double,1>& output) const;

      /**
       * Returns the input subtraction factor
       */
      inline const blitz::Array<double, 1> getInputSubraction() const
      { return m_input_sub; }

      /**
       * Sets the current input subtraction factor. We will check that the
       * number of inputs (first dimension of weights) matches the number of
       * values currently set and will raise an exception if that is not the
       * case.
       */
      void setInputSubtraction(const blitz::Array<double,1>& v);

      /**
       * Returns the input division factor
       */
      inline const blitz::Array<double, 1> getInputDivision() const
      { return m_input_div; }

      /**
       * Sets the current input division factor. We will check that the number
       * of inputs (first dimension of weights) matches the number of values
       * currently set and will raise an exception if that is not the case.
       */
      void setInputDivision(const blitz::Array<double,1>& v);

      /**
       * Returns the current weight representation. Each row should be
       * considered as a vector from which each of the output values is derived
       * by projecting the input onto such a vector.
       */
      inline const blitz::Array<double, 2> getWeights() const 
      { return m_weight; }

      /**
       * Sets the current weights. We will check that the number of inputs
       * (first dimension of weights) matches the number of biases currently
       * set and will raise an exception if that is not the case.
       */
      void setWeights(const blitz::Array<double,2>& weight);

      /**
       * Returns the biases of this classifier.
       */
      inline const blitz::Array<double, 1> getBiases() const 
      { return m_bias; }

      /**
       * Sets the current biases. We will check that the number of biases
       * matches the number of weights (first dimension) currently set and
       * will raise an exception if that is not the case.
       */
      void setBiases(const blitz::Array<double,1>& bias);

      /**
       * Returns the currently set activation function
       */
      inline Activation getActivation() const { return m_activation; }

      /**
       * Sets the activation function for each of the outputs.
       */
      void setActivation(Activation a);

      /**
       * Sets all possible vectors and matrices in one shot. With this method
       * it is possible to change the input and output sizes of the linear
       * machine in a consistent way.
       */
      void setAll(const blitz::Array<double,1> input_sub,
          const blitz::Array<double,1> input_div,
          const blitz::Array<double,2>& weight, 
          const blitz::Array<double,1>& bias);

    private: //representation

      typedef double (*actfun_t)(double); ///< activation function type

      blitz::Array<double, 1> m_input_sub; ///< input subtraction
      blitz::Array<double, 1> m_input_div; ///< input division
      blitz::Array<double, 2> m_weight; ///< weights
      blitz::Array<double, 1> m_bias; ///< biases for the output
      Activation m_activation; ///< currently set activation type
      actfun_t m_actfun; ///< currently set activation function

      mutable blitz::Array<double, 1> m_buffer; ///< a buffer for speed
  
  };

}}

#endif /* TORCH_MACHINE_LINEARMACHINE_H */
