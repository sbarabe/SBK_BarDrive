/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "SBK_BarDrive Library", "index.html", [
    [ "What's New in 2.1.2", "index.html#autotoc_md2", null ],
    [ "What's New in 2.1.1", "index.html#autotoc_md3", null ],
    [ "What's New in 2.1.0", "index.html#autotoc_md4", null ],
    [ "✨ Features", "index.html#autotoc_md6", null ],
    [ "⚙️ Supported Hardware Combinations", "index.html#autotoc_md8", [
      [ "🧩 SBK Bar Meter Boards &amp; Accessories", "index.html#autotoc_md9", null ]
    ] ],
    [ "📦 Dependencies", "index.html#autotoc_md11", null ],
    [ "⬇️ Installation", "index.html#autotoc_md12", [
      [ "Animation working memory", "index.html#autotoc_md13", null ]
    ] ],
    [ "🔊 Quick Start Examples", "index.html#autotoc_md15", [
      [ "Using MAX7219:", "index.html#autotoc_md16", null ],
      [ "Using HT16K33:", "index.html#autotoc_md17", null ],
      [ "Using a custom mapping :", "index.html#autotoc_md18", null ]
    ] ],
    [ "🎞️ Built-In Animations", "index.html#autotoc_md20", [
      [ "Fill / Empty", "index.html#autotoc_md21", null ],
      [ "Bounce Effects", "index.html#autotoc_md22", null ],
      [ "Block-Based Animations", "index.html#autotoc_md23", null ],
      [ "Signal-Driven", "index.html#autotoc_md24", null ],
      [ "Random &amp; Beat", "index.html#autotoc_md25", null ],
      [ "Static Setters", "index.html#autotoc_md26", null ]
    ] ],
    [ "🧰 Animation Helpers (Chainable)", "index.html#autotoc_md28", [
      [ "Animation queue", "index.html#autotoc_md29", null ]
    ] ],
    [ "📘 API Overview", "index.html#autotoc_md31", null ],
    [ "🪪 License", "index.html#autotoc_md33", [
      [ "Code", "index.html#autotoc_md34", null ],
      [ "Documentation", "index.html#autotoc_md35", null ]
    ] ],
    [ "🧠 Credits", "index.html#autotoc_md37", null ],
    [ "🛠️ Support", "index.html#autotoc_md39", null ],
    [ "Changelog", "md__c_h_a_n_g_e_l_o_g.html", [
      [ "[Unreleased]", "md__c_h_a_n_g_e_l_o_g.html#autotoc_md42", null ],
      [ "[2.1.2] - 2026-09-03", "md__c_h_a_n_g_e_l_o_g.html#autotoc_md43", [
        [ "Added", "md__c_h_a_n_g_e_l_o_g.html#autotoc_md44", null ],
        [ "Changed", "md__c_h_a_n_g_e_l_o_g.html#autotoc_md45", null ]
      ] ],
      [ "[2.1.1] - 2026-08-31", "md__c_h_a_n_g_e_l_o_g.html#autotoc_md46", [
        [ "Fixed", "md__c_h_a_n_g_e_l_o_g.html#autotoc_md47", null ],
        [ "Added", "md__c_h_a_n_g_e_l_o_g.html#autotoc_md48", null ]
      ] ],
      [ "[2.1.0] - 2026-08-31", "md__c_h_a_n_g_e_l_o_g.html#autotoc_md49", [
        [ "Added", "md__c_h_a_n_g_e_l_o_g.html#autotoc_md50", null ],
        [ "Changed", "md__c_h_a_n_g_e_l_o_g.html#autotoc_md51", null ],
        [ "Fixed", "md__c_h_a_n_g_e_l_o_g.html#autotoc_md52", null ]
      ] ],
      [ "Older versions", "md__c_h_a_n_g_e_l_o_g.html#autotoc_md53", null ]
    ] ],
    [ "SBK_BarDrive Roadmap", "md__r_o_a_d_m_a_p.html", [
      [ "2.1.x: stabilization", "md__r_o_a_d_m_a_p.html#autotoc_md55", null ],
      [ "3.0.0: possible internal architecture", "md__r_o_a_d_m_a_p.html#autotoc_md56", [
        [ "Typed active animation state", "md__r_o_a_d_m_a_p.html#autotoc_md57", null ],
        [ "Separate configuration, runtime, and progress", "md__r_o_a_d_m_a_p.html#autotoc_md58", null ],
        [ "Compact queue descriptors", "md__r_o_a_d_m_a_p.html#autotoc_md59", null ],
        [ "Explicit animation types and dispatch", "md__r_o_a_d_m_a_p.html#autotoc_md60", null ],
        [ "Compact flag storage", "md__r_o_a_d_m_a_p.html#autotoc_md61", null ],
        [ "Internal file organization", "md__r_o_a_d_m_a_p.html#autotoc_md62", null ],
        [ "Workspace ownership options", "md__r_o_a_d_m_a_p.html#autotoc_md63", null ],
        [ "Fading effects", "md__r_o_a_d_m_a_p.html#autotoc_md64", null ],
        [ "IS31FL3733 driver compatibility", "md__r_o_a_d_m_a_p.html#autotoc_md65", null ],
        [ "SoloDrive individual indicators", "md__r_o_a_d_m_a_p.html#autotoc_md66", null ]
      ] ],
      [ "Evaluation requirements for 3.0.0", "md__r_o_a_d_m_a_p.html#autotoc_md67", null ]
    ] ],
    [ "Data Structures", "annotated.html", [
      [ "Data Structures", "annotated.html", "annotated_dup" ],
      [ "Data Structure Index", "classes.html", null ],
      [ "Data Fields", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", null ],
        [ "Typedefs", "functions_type.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "Globals", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ],
    [ "Examples", "examples.html", "examples" ]
  ] ]
];

var NAVTREEINDEX =
[
"_s_b_k___bar_drive_8h.html",
"class_s_b_k___bar_meter_animations.html#adbb824f9be2ee96b131424471ad287bc"
];

const SYNCONMSG = 'click to disable panel synchronization';
const SYNCOFFMSG = 'click to enable panel synchronization';
const LISTOFALLMEMBERS = 'List of all members';