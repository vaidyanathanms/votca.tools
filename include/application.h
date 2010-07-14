/*
 * Copyright 2009 The VOTCA Development Team (http://www.votca.org)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __VOTCA_APPLICATION_H
#define	__VOTCA_APPLICATION_H

#include <boost/program_options.hpp>
#include "property.h"

namespace votca { namespace tools {

class Application
{
public:
    Application();
    virtual ~Application();

    /// \brief executes the program
    int Exec(int argc, char **argv);

    /// \brief program name
    ///
    /// overload this function to set the program name
    virtual string ProgramName() = 0;

    /// \brief version string of application
    virtual string VersionString();

    /// \brief help text of application without version information
    virtual void HelpText(std::ostream &out) = 0;

    /// \brief Initialize application data
    /// 
    /// Initialize is called by run before parsing the command line.
    /// All necesassary command line arguments can be added here
    virtual void Initialize() {}
    
    /// \brief Process command line options
    /// 
    /// EvaluateOptions is called by Run after parsing the command line.
    /// return true if everything is ok, false to stop and show help text.
    virtual bool EvaluateOptions() { return false; }

    /// \brief Main body of application
    ///
    /// Run is called after command line was parsed + evaluated. All
    /// the work should be done in here.
    virtual void Run() { }

    /// \brief add option for command line
    ///
    /// Adds an option to the available command line options. If no group is
    /// specified, it is added to the standard group (Allowed Options). If group
    /// is given, a sub group for this set of options will be created.
    boost::program_options::options_description_easy_init
        AddProgramOptions(const string &group = "");
    /// get available program options & descriptions
    boost::program_options::variables_map &OptionsMap() { return _op_vm; }
    boost::program_options::options_description &OptionsDesc() { return _op_desc; }

protected:
    /// Variable map containing all program options
    boost::program_options::variables_map _op_vm;

    /// program options required by all applications
    boost::program_options::options_description _op_desc;

    std::map<string, boost::program_options::options_description> _op_groups;
    
    virtual void ShowHelpText(std::ostream &out);
    
private:
    /// get input parameters from file, location may be specified in command line
    void ParseCommandLine(int argc, char **argv);
};

}}

#endif	/* __VOTCA_APPLICATION_H */

