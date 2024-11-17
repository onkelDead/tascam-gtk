/*
 Copyright 2020 Detlef Urban <onkel@paraair.de>

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

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "OConfig.h"

OConfig::OConfig() {
    config_init(&m_cfg);
    load_config();
}

OConfig::OConfig(const OConfig& orig) {
}

OConfig::~OConfig() {
    config_write_file(&m_cfg, m_conf_path);
    config_destroy(&m_cfg);
}

int OConfig::load_config() {
    
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;

    sprintf(m_conf_path, CONF_FORMATTER, homedir);
    if (!config_read_file(&m_cfg, m_conf_path)) {
        m_root = config_root_setting(&m_cfg);
        create_default();
    }
    else
        m_root = config_root_setting(&m_cfg);
    
    return 0;
}

void OConfig::create_default() {
    set_boolean(SETTINGS_WINDOW_COMPACT, false);
}
    
bool OConfig::get_boolean(const char* path) {
    config_setting_t* s;
    int ret;

    if (config_lookup_bool(&m_cfg, path, &ret) == CONFIG_TRUE) {
        return ret == 1;
    }
    s = config_setting_add(m_root, path, CONFIG_TYPE_BOOL);
    config_setting_set_bool(s, false);
    
    return false;
}

void OConfig::set_boolean(const char* path, bool val) {
    config_setting_t* s;
    
    s = config_setting_lookup(m_root, path);
    
    if (!s) {
        s = config_setting_add(m_root, path, CONFIG_TYPE_BOOL);        
    }
    config_setting_set_bool(s, val);
}

int OConfig::get_int(const char* path) {
    config_setting_t* s;
    int ret;

    if (config_lookup_int(&m_cfg, path, &ret) == CONFIG_TRUE) {
        return ret;
    }
    s = config_setting_add(m_root, path, CONFIG_TYPE_INT);
    config_setting_set_int(s, 0);
    
    return 0;
}

int OConfig::get_int(const char* path, int def_val) {
    config_setting_t* s;
    int ret;

    if (config_lookup_int(&m_cfg, path, &ret) == CONFIG_TRUE) {
        return ret;
    }
    s = config_setting_add(m_root, path, CONFIG_TYPE_INT);
    config_setting_set_int(s, def_val);
    
    return def_val;
}

void OConfig::set_int(const char* path, int val) {
    config_setting_t* s;
    
    s = config_setting_lookup(m_root, path);
    
    if (!s) {
        s = config_setting_add(m_root, path, CONFIG_TYPE_INT);        
    }
    config_setting_set_int(s, val);
}

const char* OConfig::get_string(const char* path) {
    config_setting_t* s;
    const char* ret;

    if (config_lookup_string(&m_cfg, path, &ret) == CONFIG_TRUE) {
        return ret;
    }
    s = config_setting_add(m_root, path, CONFIG_TYPE_STRING);
    config_setting_set_string(s, "");
    
    return "";
}

void OConfig::set_string(const char* path, const char* val) {
    config_setting_t* s;
    
    s = config_setting_lookup(m_root, path);
    
    if (!s) {
        s = config_setting_add(m_root, path, CONFIG_TYPE_STRING);        
    }
    config_setting_set_string(s, val);
}

void OConfig::set_string_array(const char* path, std::vector<std::string> strings) {
    // first remove list
    config_setting_t* s;
    
    s = config_setting_lookup(m_root, path);
    if (!s) {
        s = config_setting_add(m_root, path, CONFIG_TYPE_ARRAY);        
    }    
    int c = 0;
    for (std::vector<std::string>::iterator it = strings.begin(); it != strings.end(); ++it) {
        config_setting_set_string_elem(s, c, it->c_str());
    }
}