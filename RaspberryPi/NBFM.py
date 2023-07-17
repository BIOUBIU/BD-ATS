#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Not titled yet
# Author: bd-ats
# GNU Radio version: 3.10.1.1

from gnuradio import analog
from gnuradio import blocks
from gnuradio import filter
from gnuradio.filter import firdes
from gnuradio import gr
from gnuradio.fft import window
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
import osmosdr
import time
import satellites




class NBFM(gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self, "Not titled yet", catch_exceptions=True)

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 576000
        self.variable_band_pass_filter_taps_0 = variable_band_pass_filter_taps_0 = firdes.complex_band_pass(1.0, samp_rate, -10000, 10000, 200, window.WIN_HAMMING, 6.76)
        self.freq = freq = 145955000
        self.doppler_start_time = doppler_start_time = 1689516592

        ##################################################
        # Blocks
        ##################################################
        self.satellites_doppler_correction_0 = satellites.doppler_correction('/home/bd-ats/radio/doppler.txt', samp_rate, doppler_start_time)
        self.rtlsdr_source_0 = osmosdr.source(
            args="numchan=" + str(1) + " " + ""
        )
        self.rtlsdr_source_0.set_time_unknown_pps(osmosdr.time_spec_t())
        self.rtlsdr_source_0.set_sample_rate(samp_rate)
        self.rtlsdr_source_0.set_center_freq(freq, 0)
        self.rtlsdr_source_0.set_freq_corr(0, 0)
        self.rtlsdr_source_0.set_dc_offset_mode(0, 0)
        self.rtlsdr_source_0.set_iq_balance_mode(0, 0)
        self.rtlsdr_source_0.set_gain_mode(False, 0)
        self.rtlsdr_source_0.set_gain(10, 0)
        self.rtlsdr_source_0.set_if_gain(20, 0)
        self.rtlsdr_source_0.set_bb_gain(20, 0)
        self.rtlsdr_source_0.set_antenna('', 0)
        self.rtlsdr_source_0.set_bandwidth(0, 0)
        self.fft_filter_xxx_0 = filter.fft_filter_ccc(3, variable_band_pass_filter_taps_0, 1)
        self.fft_filter_xxx_0.declare_sample_delay(0)
        self.blocks_wavfile_sink_0 = blocks.wavfile_sink(
            '/home/bd-ats/radio/recording/recording.wav',
            1,
            48000,
            blocks.FORMAT_WAV,
            blocks.FORMAT_PCM_16,
            False
            )
        self.analog_simple_squelch_cc_0 = analog.simple_squelch_cc(-50, 1)
        self.analog_nbfm_rx_0 = analog.nbfm_rx(
        	audio_rate=48000,
        	quad_rate=192000,
        	tau=75e-6,
        	max_dev=5e3,
          )


        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_nbfm_rx_0, 0), (self.blocks_wavfile_sink_0, 0))
        self.connect((self.analog_simple_squelch_cc_0, 0), (self.analog_nbfm_rx_0, 0))
        self.connect((self.fft_filter_xxx_0, 0), (self.analog_simple_squelch_cc_0, 0))
        self.connect((self.rtlsdr_source_0, 0), (self.satellites_doppler_correction_0, 0))
        self.connect((self.satellites_doppler_correction_0, 0), (self.fft_filter_xxx_0, 0))


    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.set_variable_band_pass_filter_taps_0(firdes.complex_band_pass(1.0, self.samp_rate, -10000, 10000, 200, window.WIN_HAMMING, 6.76))
        self.rtlsdr_source_0.set_sample_rate(self.samp_rate)

    def get_variable_band_pass_filter_taps_0(self):
        return self.variable_band_pass_filter_taps_0

    def set_variable_band_pass_filter_taps_0(self, variable_band_pass_filter_taps_0):
        self.variable_band_pass_filter_taps_0 = variable_band_pass_filter_taps_0
        self.fft_filter_xxx_0.set_taps(self.variable_band_pass_filter_taps_0)

    def get_freq(self):
        return self.freq

    def set_freq(self, freq):
        self.freq = freq
        self.rtlsdr_source_0.set_center_freq(self.freq, 0)

    def get_doppler_start_time(self):
        return self.doppler_start_time

    def set_doppler_start_time(self, doppler_start_time):
        self.doppler_start_time = doppler_start_time




def main(top_block_cls=NBFM, options=None, endTime=9999999999.0, freq=145955000,doppler_start_time=9999999999):
    #NBFM.NBFM.set_freq(self,int(freq))
    #NBFM.NBFM.set_doppler_start_time(self,int(unixST4dp))
    tb = top_block_cls()
    tb.set_freq(freq)
    tb.set_doppler_start_time(doppler_start_time)
    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        sys.exit(0)

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    tb.start()
    while(time.time() <= endTime):
        time.sleep(0.5)
        pass
    return

    try:
        input('Press Enter to quit: ')
    except EOFError:
        pass
    tb.stop()
    tb.wait()


if __name__ == '__main__':
    main()
