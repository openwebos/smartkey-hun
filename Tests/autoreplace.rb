#!/usr/bin/ruby

# @@@LICENSE
#
#      Copyright (c) 2010-2013 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# LICENSE@@@


# sudo apt-get install libjson-ruby libjson-ruby-doc
require 'json'
require 'smartkey'

include SmartKey

# This is a test script to load an autoreplace file from the desktop and then run the
# words through the smartkey service on the device to compare it's words with the one
# in the desktop autoreplace file.

class GuessInfo
	attr_accessor :word, :result, :wordIdx, :guessIdx, :topGuess, :autoAcceptWord

	def initialize(word, result)
		self.word = word
		self.result = result
		self.wordIdx = -1
		self.guessIdx = -1
		self.topGuess = ''
		self.autoAcceptWord = ''
	end
end

def checkSpelling(word)

	query = Hash.new
	query["query"] = word

	q = JSON.generate query
	q.gsub!(/"/, '\\"')
	cmd = "luna-send -n 1 palm://com.palm.smartKey/search \"#{q}\""

	result = nil

	IO.popen(cmd) { |f|
		response = JSON.parse f.gets
		if response["returnValue"] then
			result = SpellCheckResult.new(response["spelledCorrectly"])
			guesses = response["guesses"]
			if guesses then
				guesses.each {|resultGuess|
					guess = Guess.new(resultGuess["str"])

					guess.sp = resultGuess["sp"]
					guess.autoAccept = resultGuess["auto-accept"]
					guess.autoReplace = resultGuess["auto-replace"]

					result.guesses.push guess
				}
			end
		end
	}

	return result
end

def getTopSpellingGuess(word, expectedGuess)

	result = checkSpelling(word)
	guessInfo = GuessInfo.new(word, result)

	if result != nil then
		idx = 0
		result.guesses.each do |guess|
			if guess.str == expectedGuess then
				guessInfo.guessIdx = idx
			elsif guess.str == word then
				guessInfo.wordIdx = idx
			end
			guessInfo.autoAcceptWord = guess.str if guess.autoAccept
			idx += 1
		end

		STDERR.puts "Got matching guess that was not the top guess for #{word}" if guessInfo.guessIdx > 1
		if result.guesses.length > 1 then
			guessInfo.topGuess = result.guesses[1].str
		end
	end

	return guessInfo
end

def listAutoReplaceFiles(dir)
	files = Dir.entries(dir);
	files.each {|file|
		next if not file =~ /_/
		localedir = "#{dir}/#{file}"
		next if not File.directory? localedir
		localefile = "#{localedir}/text-edit-autoreplace"
		puts localefile
	}
end

numWords = 0
numAccents = 0
numAccentsWithGuesses = 0
numCorrect = 0
numAutoAccept = 0
numAccentsSpelledCorrectly = 0
writeRawWord = false	# for Google translator toolkit

if not writeRawWord then
	puts "Input|Pre XT9|AutoAccept|spelledCorrectly|Guess IDX|Guesses"
end

dir = "autoreplace"
File.open("#{dir}/en_us/text-edit-autoreplace") { |file|
	idx = 0
	while ((line=file.gets) != nil) do
		line.chomp!
		items = line.split /\|/
		if items.length == 2 then
			numWords += 1

			word = items[0]
			mapped = items[1]

			if word =~ /^(.+)'$/ then
				idx += 1
				if false then	# Only true when wanting to test a subset of all words.
					next if idx < 900
					break if idx > 980
				end
				numAccents += 1
				if writeRawWord then
					puts mapped
				else
					info = getTopSpellingGuess($1, mapped)
					if info.result.guesses.length > 1 then
						numAccentsWithGuesses += 1
					end
					guessIdx = info.guessIdx > -1 ? info.guessIdx : ''
					if info.result.spelledCorrectly then
						numAccentsSpelledCorrectly += 1
					end
					numAutoAccept += 1 if not info.autoAcceptWord.empty?
					puts "#{info.word}|#{mapped}|#{info.autoAcceptWord}|#{info.result.spelledCorrectly}|#{guessIdx}|#{info.result.guesses_s}"
					if info.autoAcceptWord == mapped then
						numCorrect += 1
					end
				end
			end
		end
	end
}

if not writeRawWord then
	puts "Num words|#{numWords}"
	puts "Num accents|#{numAccents}"
	puts "Num accents with guesses|#{numAccentsWithGuesses}"
	puts "Num accents spelled correctly|#{numAccentsSpelledCorrectly}"
	puts "Num accents with auto-accept|#{numAutoAccept}"
	puts "Num accents with correct guesses|#{numCorrect}"
end
