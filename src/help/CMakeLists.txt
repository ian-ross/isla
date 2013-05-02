ADD_CUSTOM_TARGET(Help1 ALL
                  COMMAND tex2rtf ${CMAKE_CURRENT_SOURCE_DIR}/isla.tex isla.html -html -twice
                  DEPENDS isla.tex
                  COMMENT "Generating help files"
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

ADD_CUSTOM_TARGET(Help2 ALL
                  COMMAND bash -x -c "zip ${CMAKE_CURRENT_BINARY_DIR}/isla.htb isla*.html *.hhp *.hhk *.hhc *.gif"
                  COMMENT "Zipping help files"
                  VERBATIM
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

ADD_DEPENDENCIES(Help2 Help1)
