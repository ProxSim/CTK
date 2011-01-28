/*=========================================================================

  Library:   CTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.commontk.org/LICENSE

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/
/*=========================================================================

   Program: ParaView
   Module:    $RCSfile$

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

// Qt includes
#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QCompleter>
#include <QKeyEvent>
#include <QPointer>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QDebug>

// CTK includes
#include "ctkConsole.h"
#include "ctkConsole_p.h"
#include "ctkPimpl.h"

//-----------------------------------------------------------------------------
// ctkConsolePrivate methods

//-----------------------------------------------------------------------------
ctkConsolePrivate::ctkConsolePrivate(ctkConsole& object) :
  QTextEdit(0),
  q_ptr(&object),
  InteractivePosition(documentEnd()),
  MultilineStatement(false), Ps1("$ "), Ps2("> "),
  EditorHints(ctkConsole::AutomaticIndentation | ctkConsole::RemoveTrailingSpaces)
{
}

//-----------------------------------------------------------------------------
void ctkConsolePrivate::init()
{
  Q_Q(ctkConsole);
  this->setParent(q);
  this->setTabChangesFocus(false);
  this->setAcceptDrops(false);
  this->setAcceptRichText(false);
  this->setUndoRedoEnabled(false);

  this->PromptColor = QColor(0, 0, 0);    // Black
  this->OutputTextColor = QColor(0, 150, 0);  // Green
  this->ErrorTextColor = QColor(255, 0, 0);   // Red
  this->CommandTextColor = QColor(0, 0, 150); // Blue
  this->WelcomeTextColor = QColor(0, 0, 255); // Dark Blue

  QFont f;
  f.setFamily("Courier");
  f.setStyleHint(QFont::TypeWriter);
  f.setFixedPitch(true);

  QTextCharFormat format;
  format.setFont(f);
  format.setForeground(this->OutputTextColor);
  this->setCurrentCharFormat(format);

  this->CommandHistory.append("");
  this->CommandPosition = 0;

  QVBoxLayout * layout = new QVBoxLayout(q);
  layout->setMargin(0);
  layout->addWidget(this);
}

//-----------------------------------------------------------------------------
void ctkConsolePrivate::keyPressEvent(QKeyEvent* e)
{
  if (this->Completer && this->Completer->popup()->isVisible())
    {
    // The following keys are forwarded by the completer to the widget
    switch (e->key())
      {
      case Qt::Key_Enter:
      case Qt::Key_Return:
      case Qt::Key_Escape:
      case Qt::Key_Tab:
      case Qt::Key_Backtab:
        e->ignore();
        return; // let the completer do default behavior
      default:
        break;
      }
    }

    QTextCursor text_cursor = this->textCursor();

    // Set to true if there's a current selection
    const bool selection = text_cursor.anchor() != text_cursor.position();
    // Set to true if the cursor overlaps the history area
    const bool history_area =
      text_cursor.anchor() < this->InteractivePosition
      || text_cursor.position() < this->InteractivePosition;

    // Allow copying anywhere in the console ...
    if(e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier)
      {
      if(selection)
        {
        this->copy();
        }

      e->accept();
      return;
      }

    // Allow cut only if the selection is limited to the interactive area ...
    if(e->key() == Qt::Key_X && e->modifiers() == Qt::ControlModifier)
      {
      if(selection && !history_area)
        {
        this->cut();
        }

      e->accept();
      return;
      }

    // Allow paste only if the selection is in the interactive area ...
    if(e->key() == Qt::Key_V && e->modifiers() == Qt::ControlModifier)
      {
      if(!history_area)
        {
        const QMimeData* const clipboard = QApplication::clipboard()->mimeData();
        const QString text = clipboard->text();
        if(!text.isNull())
          {
          text_cursor.insertText(text);
          this->updateCommandBuffer();
          }
        }

      e->accept();
      return;
      }

    // Force the cursor back to the interactive area
    if(history_area && e->key() != Qt::Key_Control)
      {
      text_cursor.setPosition(this->documentEnd());
      this->setTextCursor(text_cursor);
      }

    switch(e->key())
      {
      case Qt::Key_Up:
        e->accept();

        if (this->CommandPosition > 0)
          {
          this->replaceCommandBuffer(this->CommandHistory[--this->CommandPosition]);
          }
        break;

      case Qt::Key_Down:
        e->accept();

        if (this->CommandPosition < this->CommandHistory.size() - 2)
          {
          this->replaceCommandBuffer(this->CommandHistory[++this->CommandPosition]);
          }
        else
          {
          this->CommandPosition = this->CommandHistory.size()-1;
          this->replaceCommandBuffer("");
          }
        break;

      case Qt::Key_Left:
        if (text_cursor.position() > this->InteractivePosition)
          {
          QTextEdit::keyPressEvent(e);
          }
        else
          {
          e->accept();
          }
        break;

      case Qt::Key_Delete:
        e->accept();
        QTextEdit::keyPressEvent(e);
        this->updateCommandBuffer();
        break;

      case Qt::Key_Backspace:
        e->accept();
        if(text_cursor.position() > this->InteractivePosition)
          {
          QTextEdit::keyPressEvent(e);
          this->updateCommandBuffer();
          this->updateCompleterIfVisible();
          }
        break;

      case Qt::Key_Tab:
        e->accept();
        this->updateCompleter();
        this->selectCompletion();
        break;

      case Qt::Key_Home:
        e->accept();
        text_cursor.setPosition(this->InteractivePosition);
        this->setTextCursor(text_cursor);
        break;

      case Qt::Key_Return:
      case Qt::Key_Enter:
        e->accept();

        text_cursor.setPosition(this->documentEnd());
        this->setTextCursor(text_cursor);
        this->internalExecuteCommand();
        break;

      default:
        e->accept();
        this->switchToUserInputTextColor();

        QTextEdit::keyPressEvent(e);
        this->updateCommandBuffer();
        this->updateCompleterIfVisible();
        break;
      }
}

//-----------------------------------------------------------------------------
void ctkConsolePrivate::switchToUserInputTextColor()
{
  QTextCharFormat format = this->currentCharFormat();
  format.setForeground(this->CommandTextColor);
  this->setCurrentCharFormat(format);
}

//-----------------------------------------------------------------------------
int ctkConsolePrivate::documentEnd() const
{
  QTextCursor c(this->document());
  c.movePosition(QTextCursor::End);
  return c.position();
}

//-----------------------------------------------------------------------------
void ctkConsolePrivate::focusOutEvent(QFocusEvent *e)
{
  QTextEdit::focusOutEvent(e);

  // For some reason the QCompleter tries to set the focus policy to
  // NoFocus, set let's make sure we set it back to the default WheelFocus.
  this->setFocusPolicy(Qt::WheelFocus);
}

//-----------------------------------------------------------------------------
void ctkConsolePrivate::updateCompleterIfVisible()
{
  if (this->Completer && this->Completer->popup()->isVisible())
    {
    this->updateCompleter();
    }
}

//-----------------------------------------------------------------------------
void ctkConsolePrivate::selectCompletion()
{
  if (this->Completer && this->Completer->completionCount() == 1)
    {
    this->insertCompletion(this->Completer->currentCompletion());
    this->Completer->popup()->hide();
    }
}

//-----------------------------------------------------------------------------
void ctkConsolePrivate::setCompleter(ctkConsoleCompleter* completer)
{
  if (this->Completer)
    {
    this->Completer->setWidget(0);
    disconnect(this->Completer, SIGNAL(activated(const QString&)),
               this, SLOT(insertCompletion(const QString&)));

    }
  this->Completer = completer;
  if (this->Completer)
    {
    this->Completer->setWidget(this);
    connect(this->Completer, SIGNAL(activated(const QString&)),
            this, SLOT(insertCompletion(const QString&)));
    }
}

//-----------------------------------------------------------------------------
void ctkConsolePrivate::updateCompleter()
{
  if (this->Completer)
    {
    // Get the text between the current cursor position
    // and the start of the line
    QTextCursor text_cursor = this->textCursor();
    text_cursor.setPosition(this->InteractivePosition, QTextCursor::KeepAnchor);
    QString commandText = text_cursor.selectedText();

    // Call the completer to update the completion model
    this->Completer->updateCompletionModel(commandText);

    // Place and show the completer if there are available completions
    if (this->Completer->completionCount())
      {
      // Get a QRect for the cursor at the start of the
      // current word and then translate it down 8 pixels.
      text_cursor = this->textCursor();
      text_cursor.movePosition(QTextCursor::StartOfWord);
      QRect cr = this->cursorRect(text_cursor);
      cr.translate(0,8);
      cr.setWidth(this->Completer->popup()->sizeHintForColumn(0)
        + this->Completer->popup()->verticalScrollBar()->sizeHint().width());
      this->Completer->complete(cr);
      }
    else
      {
      this->Completer->popup()->hide();
      }
    }
}

//-----------------------------------------------------------------------------c
void ctkConsolePrivate::updateCommandBuffer()
{
  this->commandBuffer() = this->toPlainText().mid(this->InteractivePosition);
}

//-----------------------------------------------------------------------------
void ctkConsolePrivate::replaceCommandBuffer(const QString& text)
{
  this->commandBuffer() = text;

  QTextCursor c(this->document());
  c.setPosition(this->InteractivePosition);
  c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
  c.removeSelectedText();
  this->switchToUserInputTextColor();
  c.setCharFormat(this->currentCharFormat());
  c.insertText(text);
}

//-----------------------------------------------------------------------------
QString& ctkConsolePrivate::commandBuffer()
{
  return this->CommandHistory.back();
}

//-----------------------------------------------------------------------------
void ctkConsolePrivate::internalExecuteCommand()
{
  Q_Q(ctkConsole);

  QString command = this->commandBuffer();

  if (this->EditorHints & ctkConsole::RemoveTrailingSpaces)
    {
    command.replace(QRegExp("\\s*$"), ""); // Remove trailing spaces
    this->commandBuffer() = command; // Update buffer
    }

  // First update the history cache. It's essential to update the
  // this->CommandPosition before calling internalExecuteCommand() since that
  // can result in a clearing of the current command (BUG #8765).
  if (!command.isEmpty()) // Don't store empty commands in the history
    {
    this->CommandHistory.push_back("");
    this->CommandPosition = this->CommandHistory.size() - 1;
    }

  QTextCursor c(this->document());
  c.movePosition(QTextCursor::End);
  c.insertText("\n");

  this->InteractivePosition = this->documentEnd();

  emit q->executing(true);
  q->executeCommand(command);
  emit q->executing(false);

  // Find the indent for the command.
  QString indent;
  if (this->EditorHints & ctkConsole::AutomaticIndentation)
    {
    QRegExp regExp("^(\\s+)");
    if (regExp.indexIn(command) != -1)
      {
      indent = regExp.cap(1);
      }
    }
  this->promptForInput(indent);
}

//-----------------------------------------------------------------------------
void ctkConsolePrivate::printString(const QString& text)
{
  this->textCursor().movePosition(QTextCursor::End);
  this->textCursor().insertText(text);
  this->InteractivePosition = this->documentEnd();
  this->ensureCursorVisible();
}

//----------------------------------------------------------------------------
void ctkConsolePrivate::printOutputMessage(const QString& text)
{
  Q_Q(ctkConsole);

  q->printMessage(text, q->outputTextColor());
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

//----------------------------------------------------------------------------
void ctkConsolePrivate::printErrorMessage(const QString& text)
{
  Q_Q(ctkConsole);

  q->printMessage(text, q->errorTextColor());
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

//-----------------------------------------------------------------------------
void ctkConsolePrivate::printCommand(const QString& cmd)
{
  this->textCursor().insertText(cmd);
  this->updateCommandBuffer();
}

//----------------------------------------------------------------------------
void ctkConsolePrivate::promptForInput(const QString& indent)
{
  Q_Q(ctkConsole);

  QTextCharFormat format = q->getFormat();
  format.setForeground(q->promptColor());
  q->setFormat(format);

  if(!this->MultilineStatement)
    {
    this->prompt(q->ps1());
    }
  else
    {
    this->prompt(q->ps2());
    }
  this->printCommand(indent);
}

//-----------------------------------------------------------------------------
void ctkConsolePrivate::prompt(const QString& text)
{
  QTextCursor text_cursor = this->textCursor();

  // If the cursor is currently on a clean line, do nothing, otherwise we move
  // the cursor to a new line before showing the prompt.
  text_cursor.movePosition(QTextCursor::StartOfLine);
  int startpos = text_cursor.position();
  text_cursor.movePosition(QTextCursor::EndOfLine);
  int endpos = text_cursor.position();
  if (endpos != startpos)
    {
    this->textCursor().insertText("\n");
    }

  this->textCursor().insertText(text);
  this->InteractivePosition = this->documentEnd();
  this->ensureCursorVisible();
}

//----------------------------------------------------------------------------
void ctkConsolePrivate::printWelcomeMessage()
{
  Q_Q(ctkConsole);

  q->printMessage(
    QLatin1String("CTK Console"),
    q->welcomeTextColor());
}

//-----------------------------------------------------------------------------
void ctkConsolePrivate::insertCompletion(const QString& completion)
{
  QTextCursor tc = this->textCursor();
  tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
  if (tc.selectedText()==".")
    {
    tc.insertText(QString(".") + completion);
    }
  else
    {
    tc = this->textCursor();
    tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
    tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    tc.insertText(completion);
    this->setTextCursor(tc);
    }
  this->updateCommandBuffer();
}

//-----------------------------------------------------------------------------
// ctkConsole methods

//-----------------------------------------------------------------------------
ctkConsole::ctkConsole(QWidget* parentObject) :
  QWidget(parentObject),
  d_ptr(new ctkConsolePrivate(*this))
{
  Q_D(ctkConsole);
  d->init();
}

//-----------------------------------------------------------------------------
ctkConsole::ctkConsole(ctkConsolePrivate * pimpl, QWidget* parentObject) :
  QWidget(parentObject), d_ptr(pimpl)
{
  Q_D(ctkConsole);
  d->init();
}

//-----------------------------------------------------------------------------
ctkConsole::~ctkConsole()
{
}

//-----------------------------------------------------------------------------
QTextCharFormat ctkConsole::getFormat() const
{
  Q_D(const ctkConsole);
  return d->currentCharFormat();
}

//-----------------------------------------------------------------------------
void ctkConsole::setFormat(const QTextCharFormat& Format)
{
  Q_D(ctkConsole);
  d->setCurrentCharFormat(Format);
}

//-----------------------------------------------------------------------------
void ctkConsole::setCompleter(ctkConsoleCompleter* completer)
{
  Q_D(ctkConsole);
  d->setCompleter(completer);
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(ctkConsole, QColor, promptColor, PromptColor);
CTK_SET_CPP(ctkConsole, const QColor&, setPromptColor, PromptColor);

//-----------------------------------------------------------------------------
CTK_GET_CPP(ctkConsole, QColor, outputTextColor, OutputTextColor);
CTK_SET_CPP(ctkConsole, const QColor&, setOutputTextColor, OutputTextColor);

//-----------------------------------------------------------------------------
CTK_GET_CPP(ctkConsole, QColor, errorTextColor, ErrorTextColor);
CTK_SET_CPP(ctkConsole, const QColor&, setErrorTextColor, ErrorTextColor);

//-----------------------------------------------------------------------------
CTK_GET_CPP(ctkConsole, QColor, commandTextColor, CommandTextColor);
CTK_SET_CPP(ctkConsole, const QColor&, setCommandTextColor, CommandTextColor);

//-----------------------------------------------------------------------------
CTK_GET_CPP(ctkConsole, QColor, welcomeTextColor, WelcomeTextColor);
CTK_SET_CPP(ctkConsole, const QColor&, setWelcomeTextColor, WelcomeTextColor);

//-----------------------------------------------------------------------------
CTK_GET_CPP(ctkConsole, QString, ps1, Ps1);
CTK_SET_CPP(ctkConsole, const QString&, setPs1, Ps1);

//-----------------------------------------------------------------------------
CTK_GET_CPP(ctkConsole, QString, ps2, Ps2);
CTK_SET_CPP(ctkConsole, const QString&, setPs2, Ps2);

//-----------------------------------------------------------------------------
CTK_GET_CPP(ctkConsole, ctkConsole::EditorHints, editorHints, EditorHints);
CTK_SET_CPP(ctkConsole, const ctkConsole::EditorHints&, setEditorHints, EditorHints);

//-----------------------------------------------------------------------------
Qt::ScrollBarPolicy ctkConsole::scrollBarPolicy()const
{
  Q_D(const ctkConsole);
  return d->verticalScrollBarPolicy();
}

//-----------------------------------------------------------------------------
void ctkConsole::setScrollBarPolicy(const Qt::ScrollBarPolicy& newScrollBarPolicy)
{
  Q_D(ctkConsole);
  d->setVerticalScrollBarPolicy(newScrollBarPolicy);
}

//-----------------------------------------------------------------------------
void ctkConsole::executeCommand(const QString& command)
{
  qWarning() << "ctkConsole::executeCommand not implemented !";
  qWarning() << "command:" << command;
}

//----------------------------------------------------------------------------
void ctkConsole::printMessage(const QString& message, const QColor& color)
{
  Q_D(ctkConsole);

  QTextCharFormat format = this->getFormat();
  format.setForeground(color);
  this->setFormat(format);
  d->printString(message);
}

//-----------------------------------------------------------------------------
void ctkConsole::clear()
{
  Q_D(ctkConsole);

  d->clear();

  // For some reason the QCompleter tries to set the focus policy to
  // NoFocus, set let's make sure we set it back to the default WheelFocus.
  d->setFocusPolicy(Qt::WheelFocus);

  d->promptForInput();
}

//-----------------------------------------------------------------------------
void ctkConsole::reset()
{
  Q_D(ctkConsole);

  d->clear();

  // For some reason the QCompleter tries to set the focus policy to
  // NoFocus, set let's make sure we set it back to the default WheelFocus.
  d->setFocusPolicy(Qt::WheelFocus);

  d->printWelcomeMessage();
  d->promptForInput();
}
