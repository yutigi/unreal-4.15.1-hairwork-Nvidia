// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Misc/InlineValue.h"
#include "MovieSceneNameableTrack.h"
#include "MovieSceneSubTrack.generated.h"

class UMovieSceneSequence;
class UMovieSceneSubSection;
struct FMovieSceneSegmentCompilerRules;

/**
 * A track that holds sub-sequences within a larger sequence.
 */
UCLASS(MinimalAPI)
class UMovieSceneSubTrack
	: public UMovieSceneNameableTrack
{
	GENERATED_BODY()

public:

	UMovieSceneSubTrack( const FObjectInitializer& ObjectInitializer );

	/**
	 * Adds a movie scene section to the end of all the existing sections. If there are no sections, the new movie scene section is added at the requested time.
	 *
	 * @param Sequence The sequence to add
	 * @param StartTime The time to add the section at
	 * @param Duration The duration of the section
	 * @param bInsertSequence Whether or not to insert the sequence and push existing sequences out
	 * @return The newly created sub section
	 */
	MOVIESCENETRACKS_API virtual UMovieSceneSubSection* AddSequence(UMovieSceneSequence* Sequence, float StartTime, float Duration, const bool& bInsertSequence = false);

	/**
	 * Check whether this track contains the given sequence.
	 *
	 * @param Sequence The sequence to find.
	 * @param Recursively Whether to search for the sequence in sub-sequences.
	 * @return true if the sequence is in this track, false otherwise.
	 */
	MOVIESCENETRACKS_API bool ContainsSequence(const UMovieSceneSequence& Sequence, bool Recursively = false) const;

	/**
	 * Add a new sequence to record
	 */
	MOVIESCENETRACKS_API virtual UMovieSceneSubSection* AddSequenceToRecord();

public:

	// UMovieSceneTrack interface

	virtual void AddSection(UMovieSceneSection& Section) override;
	virtual UMovieSceneSection* CreateNewSection() override;
	virtual const TArray<UMovieSceneSection*>& GetAllSections() const override;
	virtual TRange<float> GetSectionBoundaries() const override;
	virtual bool HasSection(const UMovieSceneSection& Section) const override;
	virtual bool IsEmpty() const override;
	virtual void RemoveAllAnimationData() override;
	virtual void RemoveSection(UMovieSceneSection& Section) override;
	virtual bool SupportsMultipleRows() const override;
	virtual void GenerateTemplate(const FMovieSceneTrackCompilerArgs& Args) const override;

#if WITH_EDITORONLY_DATA
	virtual FText GetDefaultDisplayName() const override;
#endif

protected:
	virtual TInlineValue<FMovieSceneSegmentCompilerRules> GetTrackCompilerRules() const override;

protected:

	/** All movie scene sections. */
	UPROPERTY()
	TArray<UMovieSceneSection*> Sections;
};
