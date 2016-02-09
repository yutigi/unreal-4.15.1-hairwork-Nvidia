// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

class FThumbnailSection;
class FTrackEditorThumbnail;
class FTrackEditorThumbnailPool;
class FCinematicShotTrackEditor; 
class IMenu;
class ISectionLayoutBuilder;
class UMovieSceneCinematicShotSection;
class FMovieSceneSequenceInstance;


/**
 * CinematicShot section, which paints and ticks the appropriate section.
 */
class FCinematicShotSection
	: public FThumbnailSection
{
public:

	/** Create and initialize a new instance. */
	FCinematicShotSection(TSharedPtr<ISequencer> InSequencer, TSharedPtr<FTrackEditorThumbnailPool> InThumbnailPool, UMovieSceneSection& InSection, TSharedPtr<FCinematicShotTrackEditor> InCinematicShotTrackEditor);

	/** Virtual destructor. */
	virtual ~FCinematicShotSection();

public:

	// ISequencerSection interface

	virtual int32 OnPaintSection( const FGeometry& AllottedGeometry, const FSlateRect& SectionClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, bool bParentEnabled ) const override;
	virtual void BuildSectionContextMenu(FMenuBuilder& MenuBuilder, const FGuid& ObjectBinding) override;
	virtual FText GetDisplayName() const override;
	virtual FReply OnSectionDoubleClicked(const FGeometry& SectionGeometry, const FPointerEvent& MouseEvent) override;

	// FThumbnail interface

	virtual ACameraActor* GetCameraObject() const override;
	virtual bool CanRename() const override { return true; }
	virtual FText HandleThumbnailTextBlockText() const override;
	virtual void HandleThumbnailTextBlockTextCommitted(const FText& NewThumbnailName, ETextCommit::Type CommitType) override;

private:

	/** Add shot takes menu */
	void AddTakesMenu(FMenuBuilder& MenuBuilder);

private:

	/** The section we are visualizing */
	UMovieSceneCinematicShotSection& SectionObject;

	/** The instance that this section is part of */
	TWeakPtr<FMovieSceneSequenceInstance> SequenceInstance;

	/** Sequencer interface */
	TWeakPtr<ISequencer> Sequencer;

	/** The cinematic shot track editor that contains this section */
	TWeakPtr<FCinematicShotTrackEditor> CinematicShotTrackEditor;
};