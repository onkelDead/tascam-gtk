/*
  Copyright 2017 Detlef Urban <onkel@paraair.de>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#ifndef OALSA_H
#define OALSA_H

#include <alsa/asoundlib.h>
#include <linux/version.h>


// number of input channels
#define NUM_CHANNELS 16

#define CTL_ROUTE                           "name='Line Out Route'"
#define CTL_NAME_BYPASS                     "name='DSP Bypass Switch'"
#define CTL_NAME_BUS_OUT                    "name='Buss Out Switch'"
#define CTL_NAME_MASTER_MUTE                "name='Master Mute Switch'"
#define CTL_MASTER                          "name='Master Volume'"

#define CTL_NAME_FADER                      "name='Line Volume'"
#define CTL_NAME_MUTE                       "name='Mute Switch'"
#define CTL_NAME_SOLO                       "name='Solo'"
#define CTL_NAME_PAN                        "name='Pan Left-Right Volume'"
#define CTL_NAME_PHASE                      "name='Phase Switch'"
#define CTL_NAME_EQ_ENABLE                  "name='EQ Switch'"
#define CTL_NAME_EQ_LOW_LEVEL               "name='EQ Low Volume'"
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
#define CTL_NAME_EQ_LOW_FREQ                "name='EQ Low Frequency'"
#else
#define CTL_NAME_EQ_LOW_FREQ                "name='EQ Low Frequence'"
#endif
#define CTL_NAME_EQ_MIDLOW_LEVEL            "name='EQ MidLow Volume'"
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
#define CTL_NAME_EQ_MIDLOW_FREQ             "name='EQ MidLow Frequency'"
#else
#define CTL_NAME_EQ_MIDLOW_FREQ             "name='EQ MidLow Frequence'"
#endif
#define CTL_NAME_EQ_MIDLOWWIDTH_FREQ        "name='EQ MidLow Q'"
#define CTL_NAME_EQ_MIDHIGH_LEVEL           "name='EQ MidHigh Volume'"
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
#define CTL_NAME_EQ_MIDHIGH_FREQ            "name='EQ MidHigh Frequency'"
#else
#define CTL_NAME_EQ_MIDHIGH_FREQ            "name='EQ MidHigh Frequence'"
#endif
#define CTL_NAME_EQ_MIDHIGHWIDTH_FREQ       "name='EQ MidHigh Q'"
#define CTL_NAME_EQ_HIGH_LEVEL              "name='EQ High Volume'"
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
#define CTL_NAME_EQ_HIGH_FREQ               "name='EQ High Frequency'"
#else
#define CTL_NAME_EQ_HIGH_FREQ               "name='EQ High Frequence'"
#endif
#define CTL_NAME_CP_THRESHOLD               "name='Compressor Threshold Volume'"
#define CTL_NAME_CP_GAIN                    "name='Compressor Volume'"
#define CTL_NAME_CP_ATTACK                  "name='Compressor Attack'"
#define CTL_NAME_CP_RELEASE                 "name='Compressor Release'"
#define CTL_NAME_CP_RATIO                   "name='Compressor Ratio'"
#define CTL_NAME_CP_ENABLE                  "name='Compressor Switch'"

#define RESET_VALUE_DELAY                   1000

#define CTL_NAME_CHANNEL_ACTIVE             "channel_active"
#define CTL_NAME_METER                      "name='Level Meter'"

#define CTL_NAME_INDEX_SUFFIX		    "%s,index=%d"

class OMainWnd;

class OAlsa {
public:
    OAlsa(OMainWnd*);
    virtual ~OAlsa();
    
//      worker thread function    
    void do_work(OMainWnd* caller);
    
    void stop_work();
    bool has_stopped() const;    
    
//     open alsa device
    int open_device();

//     close alsa device
    void close_device();
    
    snd_hctl_elem_t* getElement(const char* name);
    
//     get control element current value
    int getInteger(const char* name, int channel_index);
    
//     set control element integer value
    void setInteger(const char* name, int channel_index, int value);
    
//    get controls boolean value
    bool getBoolean(const char* name, int ch);
 
//    set control boolean value
    void setBoolean( const char* name, int channel_index, bool value);
    
//    get an array of integers (meter values)
    int getIntegers(const char* name, int vals[], int count);
    
    int getControlIntegers(snd_hctl_elem_t *elem, int vals[], int count);
    
    // VScale value change slot
    void on_range_control_changed (int n, const char* control_name, Gtk::VScale* control, Gtk::Label* label);
    
//    ODial value change slot
    void on_dial_control_changed (int n, const char* control_name, ODial* control);
    
//    ToggleButton set boolean value slot
    void on_toggle_button_control_changed (int n, const char* control_name, Gtk::ToggleButton* control);    
    
//    ToggleButton set boolean value slot
    void on_active_button_control_changed (int n, const char* control_name, Gtk::ToggleButton* control);    
    
//    Slot for comboBox change event
    void on_combo_control_changed (int n, const char* control_name, Gtk::ComboBoxText* control);
    
    int sliderTodB(int pos);
    int dBToSlider(int dB);
    
    
//* HCTL handle of the Tascam card 
    snd_hctl_t *hctl;    

    int meters[34];
private:

    Gtk::Window* m_caller;
        
//    * Id of the Tascam US-16x08 alsa card 
    int cardnum;
    
//     identify Tascam alsa card number
    int get_alsa_cardnum();
    
//     retrieve control element by name
    snd_hctl_elem_t* get_ctrl_by_elem(const char* name);
    
//     create control name including its index
    char* create_ctrl_elem_name(const char* name, int index, char* result[], size_t size);

    // Synchronizes access to member data.
    mutable std::mutex m_Mutex;

    // Data used by both GUI thread and worker thread.
    bool m_shall_stop;
    bool m_has_stopped;    
    
    
};

#endif /* OALSA_H */

