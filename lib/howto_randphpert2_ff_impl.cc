/* -*- c++ -*- */
/* 
 * Copyright 2017 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "howto_randphpert2_ff_impl.h"
#include <stdio.h>
#include <gnuradio/fxpt.h>

namespace gr {
  namespace transmit_nodes {

    howto_randphpert2_ff::sptr
    howto_randphpert2_ff::make(double rand_pert_size, int bf_flag)
    {
      return gnuradio::get_initial_sptr
        (new howto_randphpert2_ff_impl(rand_pert_size, bf_flag));
    }

    /*
     * The private constructor
     */
    howto_randphpert2_ff_impl::howto_randphpert2_ff_impl(double rand_pert_size, int bf_flag)
      : gr::sync_block("howto_randphpert2_ff",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(0, 0, 0)),
              d_rand_pert_size(rand_pert_size),
              d_bf_flag(bf_flag)
    {
      d_complex_gain    = (gr_complex)1;
    
      last_in_i     = 0;
      seed      = 42;
      randp       = 0;
      rand_pert_size_rad  = (3.141592654/180)*d_rand_pert_size;
      cum_ang     = 0;
      i_out     = 0;
      q_out       = 0;
      etojphi     = (gr_complex)1;
      rand_pert_from_prev_iter = 0;
    }

    /*
     * Our virtual destructor.
     */
    howto_randphpert2_ff_impl::~howto_randphpert2_ff_impl()
    {
    }

    int
    howto_randphpert2_ff_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const float *in = (const float *) input_items[0]; 
  
      for (int i = 0; i < noutput_items; i++){

      if (d_bf_flag==0){ 
          // No beamforming !
      } 
        else {
                // Beamforming !
          if (in[i] != last_in_i){
        printf ("Feedback from receiver is: %d \n", (int)in[i]);
        last_in_i = in[i];
        // Generate random angle with discrete uniform distribution
        randp = ((float)(seed.ran1()))*2;
              if (randp<1) { randp = -rand_pert_size_rad; }
              else       { randp =  rand_pert_size_rad;  }
        // Add random phase to total cumulative phase
        cum_ang += randp; 
        // If feedback bit is 0, undo previous random perturbation
              if (in[i] < 0){ cum_ang -= rand_pert_from_prev_iter; }    
        // Compute modula-2pi cumulative phase
        cum_ang = fmod(cum_ang,6.283185308);

        i_out = gr::fxpt::cos (gr::fxpt::float_to_fixed(cum_ang)); 
              q_out = gr::fxpt::sin (gr::fxpt::float_to_fixed(cum_ang));
              etojphi = gr_complex(i_out, q_out); 
              rand_pert_from_prev_iter = randp; 
        d_complex_gain = etojphi;

          } // End if(in[i] != last_in_i)
      } // End else-statement of if(d_bf_flag==0)
      } // End for(int i = 0; i < noutput_items; i++)

      return noutput_items;
    }

    void
    howto_randphpert2_ff_impl::set_bf_flag (int bf_flag)
    {
        d_bf_flag = bf_flag;
    }

  } /* namespace transmit_nodes */
} /* namespace gr */

