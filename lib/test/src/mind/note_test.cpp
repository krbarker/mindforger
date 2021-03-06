/*
 note_test.cpp     MindForger test

 Copyright (C) 2016-2018 Martin Dvorak <martin.dvorak@mindforger.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "../../../src/gear/file_utils.h"
#include "../../../src/model/outline.h"
#include "../../../src/model/outline_type.h"
#include "../../../src/model/note.h"
#include "../../../src/model/tag.h"
#include "../../../src/model/stencil.h"
#include "../../../src/model/resource_types.h"
#include "../../../src/mind/mind.h"
#include "../../../src/install/installer.h"

#include "../test_gear.h"

using namespace std;

TEST(NoteTestCase, AddNewStencilNoteToOutline) {
    // prepare M8R repository and let the mind think...
    string repositoryDir{"/tmp/mf-unit-repository-o"};
    m8r::removeDirectoryRecursively(repositoryDir.c_str());
    m8r::Installer installer{};
    installer.createEmptyMindForgerRepository(repositoryDir);
    string stencilFile{repositoryDir+"/stencils/notebooks/o-s.md"};
    string stencilContent{
        "# Stencil Test Outline"
        "\n"
        "\nOutline text."
        "\n"
        "\n## Stencil Note 1"
        "\nNote 1 text."
        "\n"
        "\n## Stencil Note 2"
        "\nNote 2 text."
        "\n"};
    m8r::stringToFile(stencilFile,stencilContent);
    stencilFile.assign(repositoryDir+"/stencils/notes/n-s.md");
    stencilContent.assign("## Stencil Single Note\nNote text.\n");
    m8r::stringToFile(stencilFile,stencilContent);
    string oFile{repositoryDir+"/memory/outline.md"};
    string oContent{"# Test Outline\n\nOutline text.\n\n## Note 1\nNote 1 text.\n\n##Note 2\nNote 2 text.\n"};
    m8r::stringToFile(oFile,oContent);

    m8r::Configuration& config = m8r::Configuration::getInstance();
    config.clear();
    config.setConfigFilePath("/tmp/cfg-ntc-ansnto.md");
    config.setActiveRepository(config.addRepository(m8r::RepositoryIndexer::getRepositoryForPath(repositoryDir)));
    m8r::Mind mind{config};
    m8r::Memory& memory = mind.remind();
    mind.learn();
    mind.think().get();

    // get the outline and add to it a stencil note
    vector<m8r::Outline*> outlines = memory.getOutlines();
    m8r::Outline* o = outlines.at(0);
    vector<const m8r::Tag*> tags{};
    tags.push_back(mind.ontology().findOrCreateTag(m8r::Tag::KeyImportant()));
    m8r::Stencil* stencil = memory.getStencils(m8r::ResourceType::NOTE).at(0);
    string name{"!!! STENCIL Note !!!"};
    mind.noteNew(
                o->getKey(),
                0,
                &name,
                mind.ontology().findOrCreateNoteType(m8r::NoteType::KeyAction()),
                0,
                &tags,
                0,
                stencil);
    // insert note in the middle and to the depth 1
    string deepName{"!!! DEEP Note !!!"};
    // insert note in the middle and to the depth 1
    m8r::Note* newNote = mind.noteNew(
                o->getKey(),
                2,
                &deepName,
                mind.ontology().findOrCreateNoteType(m8r::NoteType::KeyAction()),
                0,
                &tags,
                0,
                stencil);
    newNote->demote();
    //cout << "Begin:" << endl; for(m8r::Note* nn: o->getNotes()) { cout << "  #x" << nn->getDepth() << " " << nn->getName() << endl; }

    // asserts
    EXPECT_EQ(oFile, o->getKey());
    EXPECT_EQ(string{"Test Outline"}, o->getName());
    EXPECT_EQ(o->getNotesCount(), 4);
    EXPECT_EQ(name, o->getNotes()[0]->getName());
    EXPECT_EQ(string{"Note 1"}, o->getNotes()[1]->getName());
    EXPECT_EQ(deepName, o->getNotes()[2]->getName());
    EXPECT_EQ(string{"Note 2"}, o->getNotes()[3]->getName());

    // delete NOTE
    mind.noteForget(o->getNotes()[3]);
    //cout << "After delete:" << endl; for(m8r::Note* nn: o->getNotes()) { cout << "  #x" << nn->getDepth() << " " << nn->getName() << endl; }
    EXPECT_EQ(o->getNotesCount(), 3);
    EXPECT_EQ(name, o->getNotes()[0]->getName());
    EXPECT_EQ(string{"Note 1"}, o->getNotes()[1]->getName());
    EXPECT_EQ(deepName, o->getNotes()[2]->getName());

    // delete NOTE w/ DEEP child
    //cout << "Before DEEP delete:" << endl; for(m8r::Note* nn: o->getNotes()) { cout << "  #x" << nn->getDepth() << " " << nn->getName() << endl; }
    mind.noteForget(o->getNotes()[1]);
    //cout << "After DEEP delete:" << endl; for(m8r::Note* nn: o->getNotes()) { cout << "  #x" << nn->getDepth() << " " << nn->getName() << endl; }
    EXPECT_EQ(o->getNotesCount(), 2);
    EXPECT_EQ(name, o->getNotes()[0]->getName());

    // IMPROVE test add note on offset (virified manually that it works)
    mind.noteForget(o->getNotes()[0]);
    EXPECT_EQ(o->getNotesCount(), 0);

    mind.remind().remember(o);

    // delete OUTLINE
    EXPECT_EQ(mind.remind().getOutlinesCount(), 1);
    mind.outlineForget(o->getKey());
    EXPECT_EQ(mind.remind().getOutlinesCount(), 0);
}

TEST(NoteTestCase, PromoteDemoteUpDown) {
    // prepare M8R repository and let the mind think...
    string repositoryDir{"/tmp/mf-unit-repository-o"};
    m8r::removeDirectoryRecursively(repositoryDir.c_str());
    m8r::Installer installer{};
    installer.createEmptyMindForgerRepository(repositoryDir);
    string oFile{repositoryDir+"/memory/o.md"};
    string oContent{
        "# Note Operations Test Outline"
        "\nOutline text."
        "\n"
        "\n# 1"
        "\nT1."
        "\n"
        "\n# 2"
        "\nT2."
        "\n"
        "\n# 3"
        "\nT3."
        "\n"};
    m8r::stringToFile(oFile,oContent);

    m8r::Configuration& config = m8r::Configuration::getInstance();
    config.clear();
    config.setConfigFilePath("/tmp/cfg-ntc-pdud.md");
    config.setActiveRepository(config.addRepository(m8r::RepositoryIndexer::getRepositoryForPath(repositoryDir)));
    m8r::Mind mind{config};
    m8r::Memory& memory = mind.remind();
    mind.learn();
    mind.think().get();

    // test
    vector<m8r::Outline*> outlines = memory.getOutlines();
    m8r::Outline* o = outlines.at(0);

    m8r::Outline::Patch patch{m8r::Outline::Patch::Diff::NO,0,0};

    EXPECT_EQ(o->getNotesCount(), 3);
    EXPECT_EQ(o->getNotes()[0]->getName(), "1");
    EXPECT_EQ(o->getNotes()[1]->getName(), "2");
    EXPECT_EQ(o->getNotes()[2]->getName(), "3");
    EXPECT_EQ(o->getNotes()[0]->getDepth(), 0);
    EXPECT_EQ(o->getNotes()[1]->getDepth(), 0);
    EXPECT_EQ(o->getNotes()[2]->getDepth(), 0);

    // demote

    o->demoteNote(o->getNotes()[2], &patch);

    EXPECT_EQ(o->getNotes()[0]->getName(), "1");
    EXPECT_EQ(o->getNotes()[1]->getName(), "2");
    EXPECT_EQ(o->getNotes()[2]->getName(), "3");
    EXPECT_EQ(o->getNotes()[0]->getDepth(), 0);
    EXPECT_EQ(o->getNotes()[1]->getDepth(), 0);
    EXPECT_EQ(o->getNotes()[2]->getDepth(), 1);
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::CHANGE);
    EXPECT_EQ(patch.start, 2);
    EXPECT_EQ(patch.count, 0);

    o->demoteNote(o->getNotes()[1]);

    EXPECT_EQ(o->getNotes()[0]->getName(), "1");
    EXPECT_EQ(o->getNotes()[1]->getName(), "2");
    EXPECT_EQ(o->getNotes()[2]->getName(), "3");
    EXPECT_EQ(o->getNotes()[0]->getDepth(), 0);
    EXPECT_EQ(o->getNotes()[1]->getDepth(), 1);
    EXPECT_EQ(o->getNotes()[2]->getDepth(), 2);
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::CHANGE);
    EXPECT_EQ(patch.start, 2);
    EXPECT_EQ(patch.count, 0);

    // promote

    o->promoteNote(o->getNotes()[1], &patch);
    o->promoteNote(o->getNotes()[2], &patch);

    EXPECT_EQ(o->getNotes()[0]->getName(), "1");
    EXPECT_EQ(o->getNotes()[1]->getName(), "2");
    EXPECT_EQ(o->getNotes()[2]->getName(), "3");
    EXPECT_EQ(o->getNotes()[0]->getDepth(), 0);
    EXPECT_EQ(o->getNotes()[1]->getDepth(), 0);
    EXPECT_EQ(o->getNotes()[2]->getDepth(), 0);
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::CHANGE);
    EXPECT_EQ(patch.start, 2);
    EXPECT_EQ(patch.count, 0);

    // up

    o->moveNoteUp(o->getNotes()[1], &patch);

    EXPECT_EQ(o->getNotes()[0]->getName(), "2");
    EXPECT_EQ(o->getNotes()[1]->getName(), "1");
    EXPECT_EQ(o->getNotes()[2]->getName(), "3");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::MOVE);
    EXPECT_EQ(patch.start, 0);
    EXPECT_EQ(patch.count, 1);

    o->moveNoteUp(o->getNotes()[2], &patch);

    EXPECT_EQ(o->getNotes()[0]->getName(), "2");
    EXPECT_EQ(o->getNotes()[1]->getName(), "3");
    EXPECT_EQ(o->getNotes()[2]->getName(), "1");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::MOVE);
    EXPECT_EQ(patch.start, 1);
    EXPECT_EQ(patch.count, 1);

    o->moveNoteUp(o->getNotes()[1], &patch);

    EXPECT_EQ(o->getNotes()[0]->getName(), "3");
    EXPECT_EQ(o->getNotes()[1]->getName(), "2");
    EXPECT_EQ(o->getNotes()[2]->getName(), "1");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::MOVE);
    EXPECT_EQ(patch.start, 0);
    EXPECT_EQ(patch.count, 1);

    // down

    o->moveNoteDown(o->getNotes()[0], &patch);

    EXPECT_EQ(o->getNotes()[0]->getName(), "2");
    EXPECT_EQ(o->getNotes()[1]->getName(), "3");
    EXPECT_EQ(o->getNotes()[2]->getName(), "1");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::MOVE);
    EXPECT_EQ(patch.start, 0);
    EXPECT_EQ(patch.count, 1);

    o->moveNoteDown(o->getNotes()[1], &patch);

    EXPECT_EQ(o->getNotes()[0]->getName(), "2");
    EXPECT_EQ(o->getNotes()[1]->getName(), "1");
    EXPECT_EQ(o->getNotes()[2]->getName(), "3");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::MOVE);
    EXPECT_EQ(patch.start, 1);
    EXPECT_EQ(patch.count, 1);

    o->moveNoteDown(o->getNotes()[0], &patch);

    EXPECT_EQ(o->getNotes()[0]->getName(), "1");
    EXPECT_EQ(o->getNotes()[1]->getName(), "2");
    EXPECT_EQ(o->getNotes()[2]->getName(), "3");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::MOVE);
    EXPECT_EQ(patch.start, 0);
    EXPECT_EQ(patch.count, 1);
}

TEST(NoteTestCase, DeepUpDownFirstLastClone) {
    // prepare M8R repository and let the mind think...
    string repositoryDir{"/tmp/mf-unit-repository-o"};
    m8r::removeDirectoryRecursively(repositoryDir.c_str());
    m8r::Installer installer{};
    installer.createEmptyMindForgerRepository(repositoryDir);
    string oFile{repositoryDir+"/memory/o.md"};
    string oContent{
        "# Note Operations Test Outline"
        "\nOutline text."
        "\n"
        "\n# 1"
        "\nT1."
        "\n"
        "\n# 2"
        "\nT2."
        "\n"
        "\n# 3"
        "\nT3."
        "\n"
        "\n## 33"
        "\nT33."
        "\n"
        "\n### 333"
        "\nT333."
        "\n"
        "\n# 4"
        "\nT4."
        "\n"
        "\n## 44"
        "\nT44."
        "\n"
        "\n# 5"
        "\nT5."
        "\n"
        "\n# 6"
        "\nT6."
        "\n"
        "\n"};
    m8r::stringToFile(oFile,oContent);

    m8r::Configuration& config = m8r::Configuration::getInstance();
    config.clear();
    config.setConfigFilePath("/tmp/cfg-ntc-dudflc.md");
    config.setActiveRepository(config.addRepository(m8r::RepositoryIndexer::getRepositoryForPath(repositoryDir)));
    m8r::Mind mind{config};
    m8r::Memory& memory = mind.remind();
    mind.learn();
    mind.think().get();

    // test
    vector<m8r::Outline*> outlines = memory.getOutlines();
    m8r::Outline* o = outlines.at(0);

    m8r::Outline::Patch patch{m8r::Outline::Patch::Diff::NO,0,0};

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotesCount(), 9);
    EXPECT_EQ(o->getNotes()[0]->getName(), "1");
    EXPECT_EQ(o->getNotes()[1]->getName(), "2");
    EXPECT_EQ(o->getNotes()[2]->getName(), "3");
    EXPECT_EQ(o->getNotes()[3]->getName(), "33");
    EXPECT_EQ(o->getNotes()[4]->getName(), "333");
    EXPECT_EQ(o->getNotes()[5]->getName(), "4");
    EXPECT_EQ(o->getNotes()[6]->getName(), "44");
    EXPECT_EQ(o->getNotes()[7]->getName(), "5");
    EXPECT_EQ(o->getNotes()[8]->getName(), "6");

    // up

    o->moveNoteUp(o->getNotes()[5], &patch);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "1");
    EXPECT_EQ(o->getNotes()[1]->getName(), "2");
    EXPECT_EQ(o->getNotes()[2]->getName(), "4");
    EXPECT_EQ(o->getNotes()[3]->getName(), "44");
    EXPECT_EQ(o->getNotes()[4]->getName(), "3");
    EXPECT_EQ(o->getNotes()[5]->getName(), "33");
    EXPECT_EQ(o->getNotes()[6]->getName(), "333");
    EXPECT_EQ(o->getNotes()[7]->getName(), "5");
    EXPECT_EQ(o->getNotes()[8]->getName(), "6");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::MOVE);
    EXPECT_EQ(patch.start, 2);
    EXPECT_EQ(patch.count, 4);

    // down

    o->moveNoteDown(o->getNotes()[2], &patch);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "1");
    EXPECT_EQ(o->getNotes()[1]->getName(), "2");
    EXPECT_EQ(o->getNotes()[2]->getName(), "3");
    EXPECT_EQ(o->getNotes()[3]->getName(), "33");
    EXPECT_EQ(o->getNotes()[4]->getName(), "333");
    EXPECT_EQ(o->getNotes()[5]->getName(), "4");
    EXPECT_EQ(o->getNotes()[6]->getName(), "44");
    EXPECT_EQ(o->getNotes()[7]->getName(), "5");
    EXPECT_EQ(o->getNotes()[8]->getName(), "6");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::MOVE);
    EXPECT_EQ(patch.start, 2);
    EXPECT_EQ(patch.count, 4);

    o->moveNoteDown(o->getNotes()[5], &patch);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "1");
    EXPECT_EQ(o->getNotes()[1]->getName(), "2");
    EXPECT_EQ(o->getNotes()[2]->getName(), "3");
    EXPECT_EQ(o->getNotes()[3]->getName(), "33");
    EXPECT_EQ(o->getNotes()[4]->getName(), "333");
    EXPECT_EQ(o->getNotes()[5]->getName(), "5");
    EXPECT_EQ(o->getNotes()[6]->getName(), "4");
    EXPECT_EQ(o->getNotes()[7]->getName(), "44");
    EXPECT_EQ(o->getNotes()[8]->getName(), "6");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::MOVE);
    EXPECT_EQ(patch.start, 5);
    EXPECT_EQ(patch.count, 2);

    o->moveNoteDown(o->getNotes()[6], &patch);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "1");
    EXPECT_EQ(o->getNotes()[1]->getName(), "2");
    EXPECT_EQ(o->getNotes()[2]->getName(), "3");
    EXPECT_EQ(o->getNotes()[3]->getName(), "33");
    EXPECT_EQ(o->getNotes()[4]->getName(), "333");
    EXPECT_EQ(o->getNotes()[5]->getName(), "5");
    EXPECT_EQ(o->getNotes()[6]->getName(), "6");
    EXPECT_EQ(o->getNotes()[7]->getName(), "4");
    EXPECT_EQ(o->getNotes()[8]->getName(), "44");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::MOVE);
    EXPECT_EQ(patch.start, 6);
    EXPECT_EQ(patch.count, 2);

    o->moveNoteDown(o->getNotes()[7], &patch);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "1");
    EXPECT_EQ(o->getNotes()[1]->getName(), "2");
    EXPECT_EQ(o->getNotes()[2]->getName(), "3");
    EXPECT_EQ(o->getNotes()[3]->getName(), "33");
    EXPECT_EQ(o->getNotes()[4]->getName(), "333");
    EXPECT_EQ(o->getNotes()[5]->getName(), "5");
    EXPECT_EQ(o->getNotes()[6]->getName(), "6");
    EXPECT_EQ(o->getNotes()[7]->getName(), "4");
    EXPECT_EQ(o->getNotes()[8]->getName(), "44");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::NO);

    // up

    o->moveNoteUp(o->getNotes()[2], &patch);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "1");
    EXPECT_EQ(o->getNotes()[1]->getName(), "3");
    EXPECT_EQ(o->getNotes()[2]->getName(), "33");
    EXPECT_EQ(o->getNotes()[3]->getName(), "333");
    EXPECT_EQ(o->getNotes()[4]->getName(), "2");
    EXPECT_EQ(o->getNotes()[5]->getName(), "5");
    EXPECT_EQ(o->getNotes()[6]->getName(), "6");
    EXPECT_EQ(o->getNotes()[7]->getName(), "4");
    EXPECT_EQ(o->getNotes()[8]->getName(), "44");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::MOVE);
    EXPECT_EQ(patch.start, 1);
    EXPECT_EQ(patch.count, 3);

    o->moveNoteUp(o->getNotes()[1], &patch);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "3");
    EXPECT_EQ(o->getNotes()[1]->getName(), "33");
    EXPECT_EQ(o->getNotes()[2]->getName(), "333");
    EXPECT_EQ(o->getNotes()[3]->getName(), "1");
    EXPECT_EQ(o->getNotes()[4]->getName(), "2");
    EXPECT_EQ(o->getNotes()[5]->getName(), "5");
    EXPECT_EQ(o->getNotes()[6]->getName(), "6");
    EXPECT_EQ(o->getNotes()[7]->getName(), "4");
    EXPECT_EQ(o->getNotes()[8]->getName(), "44");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::MOVE);
    EXPECT_EQ(patch.start, 0);
    EXPECT_EQ(patch.count, 3);

    o->moveNoteUp(o->getNotes()[0], &patch);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "3");
    EXPECT_EQ(o->getNotes()[1]->getName(), "33");
    EXPECT_EQ(o->getNotes()[2]->getName(), "333");
    EXPECT_EQ(o->getNotes()[3]->getName(), "1");
    EXPECT_EQ(o->getNotes()[4]->getName(), "2");
    EXPECT_EQ(o->getNotes()[5]->getName(), "5");
    EXPECT_EQ(o->getNotes()[6]->getName(), "6");
    EXPECT_EQ(o->getNotes()[7]->getName(), "4");
    EXPECT_EQ(o->getNotes()[8]->getName(), "44");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::NO);

    // first

    o->moveNoteToFirst(o->getNotes()[4], &patch);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "2");
    EXPECT_EQ(o->getNotes()[1]->getName(), "3");
    EXPECT_EQ(o->getNotes()[2]->getName(), "33");
    EXPECT_EQ(o->getNotes()[3]->getName(), "333");
    EXPECT_EQ(o->getNotes()[4]->getName(), "1");
    EXPECT_EQ(o->getNotes()[5]->getName(), "5");
    EXPECT_EQ(o->getNotes()[6]->getName(), "6");
    EXPECT_EQ(o->getNotes()[7]->getName(), "4");
    EXPECT_EQ(o->getNotes()[8]->getName(), "44");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::MOVE);
    EXPECT_EQ(patch.start, 0);
    EXPECT_EQ(patch.count, 4);

    o->moveNoteToFirst(o->getNotes()[7], &patch);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "4");
    EXPECT_EQ(o->getNotes()[1]->getName(), "44");
    EXPECT_EQ(o->getNotes()[2]->getName(), "2");
    EXPECT_EQ(o->getNotes()[3]->getName(), "3");
    EXPECT_EQ(o->getNotes()[4]->getName(), "33");
    EXPECT_EQ(o->getNotes()[5]->getName(), "333");
    EXPECT_EQ(o->getNotes()[6]->getName(), "1");
    EXPECT_EQ(o->getNotes()[7]->getName(), "5");
    EXPECT_EQ(o->getNotes()[8]->getName(), "6");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::MOVE);
    EXPECT_EQ(patch.start, 0);
    EXPECT_EQ(patch.count, 8);

    o->moveNoteToFirst(o->getNotes()[0], &patch);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "4");
    EXPECT_EQ(o->getNotes()[1]->getName(), "44");
    EXPECT_EQ(o->getNotes()[2]->getName(), "2");
    EXPECT_EQ(o->getNotes()[3]->getName(), "3");
    EXPECT_EQ(o->getNotes()[4]->getName(), "33");
    EXPECT_EQ(o->getNotes()[5]->getName(), "333");
    EXPECT_EQ(o->getNotes()[6]->getName(), "1");
    EXPECT_EQ(o->getNotes()[7]->getName(), "5");
    EXPECT_EQ(o->getNotes()[8]->getName(), "6");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::NO);

    // last

    o->moveNoteToLast(o->getNotes()[6], &patch);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "4");
    EXPECT_EQ(o->getNotes()[1]->getName(), "44");
    EXPECT_EQ(o->getNotes()[2]->getName(), "2");
    EXPECT_EQ(o->getNotes()[3]->getName(), "3");
    EXPECT_EQ(o->getNotes()[4]->getName(), "33");
    EXPECT_EQ(o->getNotes()[5]->getName(), "333");
    EXPECT_EQ(o->getNotes()[6]->getName(), "5");
    EXPECT_EQ(o->getNotes()[7]->getName(), "6");
    EXPECT_EQ(o->getNotes()[8]->getName(), "1");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::MOVE);
    EXPECT_EQ(patch.start, 6);
    EXPECT_EQ(patch.count, 2);

    o->moveNoteToLast(o->getNotes()[3], &patch);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "4");
    EXPECT_EQ(o->getNotes()[1]->getName(), "44");
    EXPECT_EQ(o->getNotes()[2]->getName(), "2");
    EXPECT_EQ(o->getNotes()[3]->getName(), "5");
    EXPECT_EQ(o->getNotes()[4]->getName(), "6");
    EXPECT_EQ(o->getNotes()[5]->getName(), "1");
    EXPECT_EQ(o->getNotes()[6]->getName(), "3");
    EXPECT_EQ(o->getNotes()[7]->getName(), "33");
    EXPECT_EQ(o->getNotes()[8]->getName(), "333");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::MOVE);
    EXPECT_EQ(patch.start, 3);
    EXPECT_EQ(patch.count, 5);

    o->moveNoteToLast(o->getNotes()[6], &patch);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "4");
    EXPECT_EQ(o->getNotes()[1]->getName(), "44");
    EXPECT_EQ(o->getNotes()[2]->getName(), "2");
    EXPECT_EQ(o->getNotes()[3]->getName(), "5");
    EXPECT_EQ(o->getNotes()[4]->getName(), "6");
    EXPECT_EQ(o->getNotes()[5]->getName(), "1");
    EXPECT_EQ(o->getNotes()[6]->getName(), "3");
    EXPECT_EQ(o->getNotes()[7]->getName(), "33");
    EXPECT_EQ(o->getNotes()[8]->getName(), "333");
    EXPECT_EQ(patch.diff, m8r::Outline::Patch::Diff::NO);

    // clone

    o->cloneNote(o->getNotes()[3]);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "4");
    EXPECT_EQ(o->getNotes()[1]->getName(), "44");
    EXPECT_EQ(o->getNotes()[2]->getName(), "2");
    EXPECT_EQ(o->getNotes()[3]->getName(), "5");
    EXPECT_EQ(o->getNotes()[4]->getName(), "5");
    EXPECT_EQ(o->getNotes()[5]->getName(), "6");
    EXPECT_EQ(o->getNotes()[6]->getName(), "1");
    EXPECT_EQ(o->getNotes()[7]->getName(), "3");
    EXPECT_EQ(o->getNotes()[8]->getName(), "33");
    EXPECT_EQ(o->getNotes()[9]->getName(), "333");
    // Note data
    EXPECT_EQ(o->getNotes()[4]->getDepth(), 0);
    EXPECT_EQ(o->getNotes()[4]->getReads(), 1);
    EXPECT_EQ(o->getNotes()[4]->getRevision(), 1);

    o->cloneNote(o->getNotes()[0]);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "4");
    EXPECT_EQ(o->getNotes()[1]->getName(), "44");
    EXPECT_EQ(o->getNotes()[2]->getName(), "4");
    EXPECT_EQ(o->getNotes()[3]->getName(), "44");
    EXPECT_EQ(o->getNotes()[4]->getName(), "2");
    EXPECT_EQ(o->getNotes()[5]->getName(), "5");
    EXPECT_EQ(o->getNotes()[6]->getName(), "5");
    EXPECT_EQ(o->getNotes()[7]->getName(), "6");
    EXPECT_EQ(o->getNotes()[8]->getName(), "1");
    EXPECT_EQ(o->getNotes()[9]->getName(), "3");
    EXPECT_EQ(o->getNotes()[10]->getName(), "33");
    EXPECT_EQ(o->getNotes()[11]->getName(), "333");
    // Note data
    EXPECT_EQ(o->getNotes()[2]->getDepth(), 0);
    EXPECT_EQ(o->getNotes()[2]->getReads(), 1);
    EXPECT_EQ(o->getNotes()[2]->getRevision(), 1);
    EXPECT_EQ(o->getNotes()[3]->getDepth(), 1);
    EXPECT_EQ(o->getNotes()[3]->getReads(), 1);
    EXPECT_EQ(o->getNotes()[3]->getRevision(), 1);

    o->cloneNote(o->getNotes()[9]);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "4");
    EXPECT_EQ(o->getNotes()[1]->getName(), "44");
    EXPECT_EQ(o->getNotes()[2]->getName(), "4");
    EXPECT_EQ(o->getNotes()[3]->getName(), "44");
    EXPECT_EQ(o->getNotes()[4]->getName(), "2");
    EXPECT_EQ(o->getNotes()[5]->getName(), "5");
    EXPECT_EQ(o->getNotes()[6]->getName(), "5");
    EXPECT_EQ(o->getNotes()[7]->getName(), "6");
    EXPECT_EQ(o->getNotes()[8]->getName(), "1");
    EXPECT_EQ(o->getNotes()[9]->getName(), "3");
    EXPECT_EQ(o->getNotes()[10]->getName(),"33");
    EXPECT_EQ(o->getNotes()[11]->getName(),"333");
    EXPECT_EQ(o->getNotes()[12]->getName(),"3");
    EXPECT_EQ(o->getNotes()[13]->getName(),"33");
    EXPECT_EQ(o->getNotes()[14]->getName(),"333");
    // Note data
    EXPECT_EQ(o->getNotes()[12]->getDepth(), 0);
    EXPECT_EQ(o->getNotes()[12]->getReads(), 1);
    EXPECT_EQ(o->getNotes()[12]->getRevision(), 1);
    EXPECT_EQ(o->getNotes()[13]->getDepth(), 1);
    EXPECT_EQ(o->getNotes()[13]->getReads(), 1);
    EXPECT_EQ(o->getNotes()[13]->getRevision(), 1);
    EXPECT_EQ(o->getNotes()[14]->getDepth(), 2);
    EXPECT_EQ(o->getNotes()[14]->getReads(), 1);
    EXPECT_EQ(o->getNotes()[14]->getRevision(), 1);

    o->cloneNote(o->getNotes()[13]);

    //printOutlineNotes(o);
    EXPECT_EQ(o->getNotes()[0]->getName(), "4");
    EXPECT_EQ(o->getNotes()[1]->getName(), "44");
    EXPECT_EQ(o->getNotes()[2]->getName(), "4");
    EXPECT_EQ(o->getNotes()[3]->getName(), "44");
    EXPECT_EQ(o->getNotes()[4]->getName(), "2");
    EXPECT_EQ(o->getNotes()[5]->getName(), "5");
    EXPECT_EQ(o->getNotes()[6]->getName(), "5");
    EXPECT_EQ(o->getNotes()[7]->getName(), "6");
    EXPECT_EQ(o->getNotes()[8]->getName(), "1");
    EXPECT_EQ(o->getNotes()[9]->getName(), "3");
    EXPECT_EQ(o->getNotes()[10]->getName(),"33");
    EXPECT_EQ(o->getNotes()[11]->getName(),"333");
    EXPECT_EQ(o->getNotes()[12]->getName(),"3");
    EXPECT_EQ(o->getNotes()[13]->getName(),"33");
    EXPECT_EQ(o->getNotes()[14]->getName(),"333");
    EXPECT_EQ(o->getNotes()[15]->getName(),"33");
    EXPECT_EQ(o->getNotes()[16]->getName(),"333");
    // Note data
    EXPECT_EQ(o->getNotes()[15]->getDepth(), 1);
    EXPECT_EQ(o->getNotes()[15]->getReads(), 1);
    EXPECT_EQ(o->getNotes()[15]->getRevision(), 1);
    EXPECT_EQ(o->getNotes()[16]->getDepth(), 2);
    EXPECT_EQ(o->getNotes()[16]->getReads(), 1);
    EXPECT_EQ(o->getNotes()[16]->getRevision(), 1);
}


TEST(NoteTestCase, RefactorNote) {
    // prepare M8R repository and let the mind think...
    string repositoryDir{"/tmp/mf-unit-repository-refactor"};
    m8r::removeDirectoryRecursively(repositoryDir.c_str());
    m8r::Installer installer{};
    installer.createEmptyMindForgerRepository(repositoryDir);
    // source O
    string sFile{repositoryDir+"/memory/source.md"};
    string sContent{
        "# Source Test Outline"
        "\nOutline text."
        "\n"
        "\n# 1"
        "\nT1."
        "\n"
        "\n# 2"
        "\nT2."
        "\n"
        "\n# 3"
        "\nT3."
        "\n"
        "\n## 33"
        "\nT33."
        "\n"
        "\n### 333"
        "\nT333."
        "\n"
        "\n# 4"
        "\nT4."
        "\n"
        "\n## 44"
        "\nT44."
        "\n"
        "\n# 5"
        "\nT5."
        "\n"
        "\n# 6"
        "\nT6."
        "\n"
        "\n"};
    m8r::stringToFile(sFile,sContent);

    // target O
    string tFile{repositoryDir+"/memory/target.md"};
    string tContent{
        "# Target Test Outline"
        "\nOutline text."
        "\n"
        "\n# A"
        "\nTA."
        "\n"
        "\n# B"
        "\nTB."
        "\n"
        "\n# C"
        "\nTC."
        "\n"
        "\n## CC"
        "\nTCC."
        "\n"
        "\n### CCC"
        "\nTCCC."
        "\n"
        "\n# D"
        "\nTD."
        "\n"
        "\n## DD"
        "\nTDD."
        "\n"
        "\n# E"
        "\nTE."
        "\n"
        "\n# F"
        "\nTF."
        "\n"
        "\n"};
    m8r::stringToFile(tFile,tContent);

    m8r::Configuration& config = m8r::Configuration::getInstance();
    config.clear();
    config.setConfigFilePath("/tmp/cfg-ntc-rn.md");
    config.setActiveRepository(config.addRepository(m8r::RepositoryIndexer::getRepositoryForPath(repositoryDir)));
    m8r::Mind mind{config};
    mind.learn();
    mind.think().get();

    // test
    m8r::Outline* s = mind.remind().getOutline(sFile);
    m8r::Outline* t = mind.remind().getOutline(tFile);
    EXPECT_TRUE(s != nullptr);
    EXPECT_TRUE(t != nullptr);
    EXPECT_EQ(mind.remind().getOutlinesCount(), 2);
    EXPECT_EQ(mind.remind().getNotesCount(), 18);

    EXPECT_EQ(s->getNotes()[2]->getName(), "3");

    mind.noteRefactor(s->getNotes()[2], t->getKey());

    // asserts
    EXPECT_EQ(s->getNotesCount(), 9-3);
    EXPECT_EQ(s->getNotes()[0]->getName(), "1");
    EXPECT_EQ(s->getNotes()[1]->getName(), "2");
    EXPECT_EQ(s->getNotes()[2]->getName(), "4");
    EXPECT_EQ(t->getNotesCount(), 9+3);
    EXPECT_EQ(t->getNotes()[0]->getName(), "3");
    EXPECT_EQ(t->getNotes()[1]->getName(), "33");
    EXPECT_EQ(t->getNotes()[2]->getName(), "333");
    EXPECT_EQ(t->getNotes()[3]->getName(), "A");

    // IMPROVE assert Os and Ns modified
}

TEST(NoteTestCase, MangleNoteName) {
    string repositoryDir{"/tmp/mf-unit-repository-m"};
    m8r::removeDirectoryRecursively(repositoryDir.c_str());
    m8r::Installer installer{};
    installer.createEmptyMindForgerRepository(repositoryDir);
    string oFile{repositoryDir+"/memory/o.md"};
    string oContent{
        "# Section Links Mangling"
        "\n"
        "\nThis document elaborates mangling of section names to links."
        "\n"
        "\n# Section with space"
        "\n`section-with-space`"
        "\n"
        "\n# Section with 123 number"
        "\n`section-with-123-number`"
        "\n"
        "\n# Question?"
        "\n`question`"
        "\n"
        "\n#   Leading    within and trailing spaces"
        "\n`leading----within-and-trailing-spaces`"
        "\n"
        "\n# ?"
        "\n`` ... empty"
        "\n"
        "\n# @$%^&*("
        "\n`-1` ... probably GitHub bug (should be empty string)"
        "\n"};
    m8r::stringToFile(oFile,oContent);

    m8r::Configuration& config = m8r::Configuration::getInstance();
    config.clear();
    config.setConfigFilePath("/tmp/cfg-ntc-mnn.md");
    config.setActiveRepository(config.addRepository(m8r::RepositoryIndexer::getRepositoryForPath(repositoryDir)));
    m8r::Mind mind{config};
    m8r::Memory& memory = mind.remind();
    mind.learn();
    mind.think().get(); // ensure that ASYNC learning finishes

    // test
    vector<m8r::Outline*> outlines = memory.getOutlines();
    m8r::Outline* o = outlines.at(0);

    EXPECT_EQ(o->getNotesCount(), 6);
    EXPECT_EQ("section-with-space", o->getNotes()[0]->getMangledName());
    EXPECT_EQ("section-with-123-number", o->getNotes()[1]->getMangledName());
    EXPECT_EQ("question", o->getNotes()[2]->getMangledName());
    EXPECT_EQ("leading----within-and-trailing-spaces", o->getNotes()[3]->getMangledName());
    EXPECT_EQ("", o->getNotes()[4]->getMangledName());
    EXPECT_EQ("", o->getNotes()[5]->getMangledName());
}
