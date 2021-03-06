/*
  NUI3 - C++ cross-platform GUI framework for OpenGL based applications
  Copyright (C) 2002-2003 Sebastien Metrot

  licence: see nui3/LICENCE.TXT
*/

#include "nui.h"
#include "nuiKnob.h"
#include "nuiDrawContext.h"
#include "nuiXML.h"

#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
nuiSize nuiKnob::mDefaultSensitivity = 4;
#else
nuiSize nuiKnob::mDefaultSensitivity = 10;
#endif
nuiSize nuiKnob::mDefaultFineSensitivityRatio = 10;
nglKeyCode nuiKnob::mDefaultFineSensitivityKey = NK_LSHIFT;

nuiKnob::nuiKnob(const nuiRange& rRange)
  : nuiSimpleContainer(),
    mKnobSink(this)
{
  if (SetObjectClass(_T("nuiKnob")))
    InitAttributes();
  Init(rRange);
}

nuiKnob::nuiKnob(const nuiRange& rRange, nuiImageSequence* pImageSequence)
: nuiSimpleContainer(),
mKnobSink(this)
{
  if (SetObjectClass(_T("nuiKnob")))
    InitAttributes();
  Init(rRange);
  SetImageSequence(pImageSequence);
}


void nuiKnob::Init(const nuiRange& rRange)
{
  mRange = rRange;
  mpImageSequence = NULL;
  
  mClicked = false;
  mRange.SetPageSize(0);
  mFrameIndex = 0;
  
  mSensitivity = mDefaultSensitivity;
  mFineSensitivityRatio = mDefaultFineSensitivityRatio;
  mFineSensitivityKey = mDefaultFineSensitivityKey;
  
  mKnobSink.Connect(mRange.Changed, &nuiKnob::DoInvalidate);
  //mKnobSink.Connect(mRange.ValueChanged, &nuiKnob::DoInvalidate);
  NUI_ADD_EVENT(ValueChanged);
  NUI_ADD_EVENT(InteractiveValueChanged);  
  
  mSequenceNeedRefresh = false;
  mSequenceNbFrames = 0;
  mSequenceOrientation = nuiVertical;
  
  SetWantKeyboardFocus(true);
}


void nuiKnob::InitAttributes()
{
  AddAttribute(new nuiAttribute<const nglPath&>
  (nglString(_T("Sequence")), nuiUnitNone,
   nuiMakeDelegate(this, &nuiKnob::GetSequencePath), 
   nuiMakeDelegate(this, &nuiKnob::SetSequencePath)));
  
  AddAttribute(new nuiAttribute<uint32>
  (nglString(_T("NbFrames")), nuiUnitNone,
   nuiMakeDelegate(this, &nuiKnob::GetNbFrames),
   nuiMakeDelegate(this, &nuiKnob::SetNbFrames)));
  
  AddAttribute(new nuiAttribute<nuiOrientation>
  (nglString(_T("Orientation")), nuiUnitNone,
   nuiMakeDelegate(this, &nuiKnob::GetOrientation),
   nuiMakeDelegate(this, &nuiKnob::SetOrientation)));
  
}  


// attributes
const nglPath& nuiKnob::GetSequencePath() const
{
  return mpImageSequence->GetTexturePath();
}

void nuiKnob::SetSequencePath(const nglPath& rPath)
{
  mSequencePath = rPath;
  mSequenceNeedRefresh = true;
  nuiEvent event;
  DoInvalidate(event);
}


nuiOrientation nuiKnob::GetOrientation() const
{
  return mpImageSequence->GetOrientation();
}


void nuiKnob::SetOrientation(nuiOrientation orientation)
{
  mSequenceOrientation = orientation;
  mSequenceNeedRefresh = true;
  nuiEvent event;
  DoInvalidate(event);
}


uint32 nuiKnob::GetNbFrames() const
{
  return mpImageSequence->GetNbFrames();
}


void nuiKnob::SetNbFrames(uint32 nbFrames)
{
  mSequenceNbFrames = nbFrames;
  mSequenceNeedRefresh = true;
  nuiEvent event;
  DoInvalidate(event);
}





bool nuiKnob::Load(const nuiXMLNode* pNode)
{
  nuiSimpleContainer::Load(pNode);

  SetObjectClass(_T("nuiKnob"));
  mClicked = false;
  mInteractiveValueChanged = false;

  mSensitivity = mDefaultSensitivity;
  mFineSensitivityRatio = mDefaultFineSensitivityRatio;
  mFineSensitivityKey = mDefaultFineSensitivityKey;

  // FIXME: interpret other attributes...
  mKnobSink.Connect(mRange.Changed, &nuiKnob::DoInvalidate);
  //mKnobSink.Connect(mRange.ValueChanged, &nuiKnob::DoInvalidate);
  NUI_ADD_EVENT(ValueChanged);
  NUI_ADD_EVENT(InteractiveValueChanged);
  
  return true;
}

nuiXMLNode* nuiKnob::Serialize(nuiXMLNode* pParentNode, bool Recursive) const
{
  nuiXMLNode* pNode = nuiWidget::Serialize(pParentNode,true);
  if (!pNode) 
    return NULL;

  return pNode;
}


nuiKnob::~nuiKnob()
{
  if (mpImageSequence)
    mpImageSequence->Release();
}


void nuiKnob::SetImageSequence(nuiImageSequence* pImageSequence)
{
  if (pImageSequence)
    pImageSequence->Acquire();
  if (mpImageSequence)
    mpImageSequence->Release();
  mpImageSequence = pImageSequence;
}

nuiImageSequence* nuiKnob::GetImageSequence() const
{
  return mpImageSequence;
}


// Keyboard events:
bool nuiKnob::KeyDown(const nglKeyEvent& rEvent)
{
  if (IsKeyDown(mFineSensitivityKey))
  {
    if (rEvent.mKey == NK_DOWN || rEvent.mKey == NK_LEFT)
    {
      mRange.SetValue(mRange.GetValue() - mRange.GetIncrement() / mFineSensitivityRatio);
      InteractiveValueChanged();
      ActivateToolTip(this, true);
      return true;
    }
    if (rEvent.mKey == NK_UP || rEvent.mKey == NK_RIGHT)
    {
      mRange.SetValue(mRange.GetValue() + mRange.GetIncrement() / mFineSensitivityRatio);
      InteractiveValueChanged();
      ActivateToolTip(this, true);
      return true;
    }
  }
  else
  {
    if (rEvent.mKey == NK_DOWN || rEvent.mKey == NK_LEFT)
    {
      mRange.Decrement();
      InteractiveValueChanged();
      ActivateToolTip(this, true);
      return true;
    }
    if (rEvent.mKey == NK_UP || rEvent.mKey == NK_RIGHT)
    {
      mRange.Increment();
      InteractiveValueChanged();
      ActivateToolTip(this, true);
      return true;
    }
  }
  
  if (rEvent.mKey == NK_PAGEDOWN)
  {
    mRange.PageIncrement();
    InteractiveValueChanged();
    ActivateToolTip(this, true);
    return true;
  }
  if (rEvent.mKey == NK_PAGEDOWN)
  {
    mRange.PageDecrement();
    InteractiveValueChanged();
    ActivateToolTip(this, true);
    return true;
  }

  if (rEvent.mKey == NK_HOME)
  {
    double v = mRange.GetMinimum();
    mRange.SetValue(v);
    InteractiveValueChanged();
    ActivateToolTip(this, true);
    return true;
  }
  if (rEvent.mKey == NK_END)
  {
    double v = mRange.GetMaximum();
    mRange.SetValue(v);
    InteractiveValueChanged();
    ActivateToolTip(this, true);
    return true;
  }
  
  
  return false;
}

bool nuiKnob::KeyUp(const nglKeyEvent& rEvent)
{
  if (rEvent.mKey == NK_DOWN || rEvent.mKey == NK_LEFT || rEvent.mKey == NK_UP || rEvent.mKey == NK_RIGHT || rEvent.mKey == NK_PAGEDOWN || rEvent.mKey == NK_PAGEDOWN)
    return true;
  
  return false;
}



// Received Mouse events:
bool nuiKnob::MouseClicked(nuiSize X, nuiSize Y, nglMouseInfo::Flags Button)
{
  mClickX = X;
  mClickY = Y;

  if ((Button & nglMouseInfo::ButtonLeft) && (Button & nglMouseInfo::ButtonDoubleClick))
  {
    return false;
  }
  else if (Button & nglMouseInfo::ButtonLeft)
  {
    mClicked = true;
    Grab();
    Invalidate();
    mClickValue = mRange.GetValue();
    
    return true;
  }
  else if (Button & nglMouseInfo::ButtonWheelUp)
  {
    if (IsKeyDown(mFineSensitivityKey))
    {
      mRange.SetValue(mRange.GetValue() + mRange.GetIncrement() / mFineSensitivityRatio);
    }
    else
    {
      mRange.Increment();
    }
    InteractiveValueChanged();
    ActivateToolTip(this, true);
    return true;
  }
  else if (Button & nglMouseInfo::ButtonWheelDown)
  {
    if (IsKeyDown(mFineSensitivityKey))
    {
      mRange.SetValue(mRange.GetValue() - mRange.GetIncrement() / mFineSensitivityRatio);
    }
    else
    {
      mRange.Decrement();
    }
    InteractiveValueChanged();
    ActivateToolTip(this, true);
    return true;
  }
  return false;
}            

bool nuiKnob::MouseUnclicked(nuiSize X, nuiSize Y, nglMouseInfo::Flags Button)
{
  if (mInteractiveValueChanged)
    ValueChanged();
  mInteractiveValueChanged = false;
  
  if (Button & nglMouseInfo::ButtonLeft)
  {
    mClicked = false;

    Ungrab();
    
    Invalidate();
    return true;
  }
  return false;
}

bool nuiKnob::MouseMoved(nuiSize X, nuiSize Y)
{
  if (mClicked)
  {
    nuiSize sensitivity = mSensitivity;
    if (IsKeyDown(mFineSensitivityKey))
    {
      sensitivity *= mFineSensitivityRatio;
    }

    nuiSize x,y;
    nuiSize range = sqrtf(GetRect().GetWidth() * GetRect().GetHeight());
    x = X-mClickX;
    y = mClickY - Y;

    //nuiSize length = range;
    nuiSize start= mClickValue;
    nuiSize movement = ( x + y ) / sensitivity;

    start += (mRange.GetRange() - mRange.GetPageSize()) * (movement/range);
    
    mRange.SetValue(start);
    
    mInteractiveValueChanged = true;
    InteractiveValueChanged();
    
    // Rotate the contained widget if there is only one:
    /* unused: nuiWidget* pItem = NULL; */

    long count = mpChildren.size();
    if (!count)
      return true;

    return true;
  }

  return false;
}

// Data management:
void nuiKnob::HandlePageDown(const nuiEvent& rEvent)
{
  mRange.PageIncrement();
}

void nuiKnob::HandlePageUp(const nuiEvent& rEvent)
{
  mRange.PageDecrement();
}

nuiRange& nuiKnob::GetRange()
{
  return mRange;
}



void nuiKnob::DoInvalidate(const nuiEvent& rEvent)
{
  if (mSequenceNeedRefresh)
  {
    if (mpImageSequence)
      mpImageSequence->Release();
    mpImageSequence = NULL;
    
    mpImageSequence = new nuiImageSequence();
    mpImageSequence->SetNbFrames(mSequenceNbFrames);
    mpImageSequence->SetTexturePath(mSequencePath);
    mpImageSequence->SetOrientation(mSequenceOrientation);
    mSequenceNeedRefresh = false;
  }
  
  Invalidate();
}

nuiRect nuiKnob::CalcIdealSize()
{
  if (mpChildren.empty())
  {
    if (!mpImageSequence)
      return nuiRect(0,0,0,0);
    
    const nuiRect& rect = mpImageSequence->CalcIdealSize();
    return rect;
  }
  else
  {
    return nuiSimpleContainer::CalcIdealSize();
  }
}

bool nuiKnob::SetRect(const nuiRect& rRect)
{
  nuiWidget::SetRect(rRect);
  return true;
}

bool nuiKnob::Draw(nuiDrawContext* pContext)
{
  if (mpImageSequence)
  {
    mFrameIndex = (int)((mpImageSequence->GetNbFrames()-1) * (mRange.GetValue() - mRange.GetMinimum())) / (mRange.GetMaximum() - mRange.GetMinimum());
    
    mpImageSequence->SetFrameIndex(mFrameIndex);
    mpImageSequence->Draw(pContext, this);
  }
  
  return nuiSimpleContainer::Draw(pContext);
  
}

void nuiKnob::SetDefaultSensitivity(nuiSize DefaultSensitivity) 
{ 
  mDefaultSensitivity = DefaultSensitivity;
}

void nuiKnob::SetSensitivity(nuiSize Sensitivity) 
{ 
  mSensitivity = Sensitivity; 
} 

// the bigger the ratio is, the slower the knob will turn.
void nuiKnob::SetDefaultFineSensitivityRatio(nuiSize DefaultFineSensitivityRatio) 
{ 
  mDefaultFineSensitivityRatio = DefaultFineSensitivityRatio;
}

void nuiKnob::SetFineSensitivityRatio(nuiSize FineSensitivityRatio) 
{ 
  mFineSensitivityRatio = FineSensitivityRatio; 
} 

void nuiKnob::SetDefaultFineSensitivityKey(nglKeyCode DefaultFineSensitivityKey)
{
  mDefaultFineSensitivityKey = DefaultFineSensitivityKey;
}

void nuiKnob::SetFineSensitivityKey(nglKeyCode FineSensitivityKey)
{
  mFineSensitivityKey = FineSensitivityKey;
}

