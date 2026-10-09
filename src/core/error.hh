#pragma once

enum Error { OK, FAILED, ERR_CANT_CREATE };

#define ERR_FAIL_COND_V(m_cond, m_retval) \
    do {                                  \
        if (m_cond) {                     \
            return m_retval;              \
        }                                 \
    } while (0)
