#!/usr/bin/ruby

# @@@LICENSE
#
#      Copyright (c) 2010 - 2012 Hewlett-Packard Development Company, L.P.
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

# This is a simple unit test for spell corrections. To run it ou
# will need to be tethered to the device - aka
# 
# ls-control enable-tether
#
# So this test runs against the smartkey service running on the device

require 'smartkey'

include SmartKey

$proxy = SmartKeyProxy.new


class TestInfo
	attr_accessor :valid, :word, :spelledCorrectly, :topGuess, :autoReplace, :autoAccept
	def initialize(line)
		line.chomp!
		items = line.split /,/
		if items.length == 5 then
			self.valid = true
			self.word = items[0]
			self.spelledCorrectly = items[1] == 'true'
			self.topGuess = items[2]
			self.autoReplace = items[3] == 'true'
			self.autoAccept = items[4] == 'true'
		else
			self.valid = false
			self.word = ''
			self.spelledCorrectly = false
			self.topGuess = ''
			self.autoReplace = false
			self.autoAccept = false
		end
	end

	def == (rhs)
		return self.word == rhs.word &&
			self.spelledCorrectly == rhs.spelledCorrectly &&
			self.topGuess == rhs.topGuess &&
			self.autoReplace == rhs.autoReplace &&
			self.autoAccept == rhs.autoAccept
	end

	def to_s
		return "word:\"#{self.word}\", sc:#{self.spelledCorrectly}, top:\"#{self.topGuess}\", ar:#{self.autoReplace}, aa:#{self.autoAccept}"
	end

	def valid?
		return self.valid
	end
end

class ResultInfo
	attr_accessor :result, :testInfo

	def initialize
		self.result = nil
		self.testInfo = TestInfo.new('')
	end

	##
	# Process the SmartKey service response identically to how WebKit does it.
	# Select a topGuess (winning candidate).
	#
	def queryAndProccessResult(word)
		self.result = $proxy.checkSpelling(word);
		self.testInfo.valid = true
		self.testInfo.word = word
		self.testInfo.spelledCorrectly = self.result.spelledCorrectly

		# First look for the auto replace entries
		self.result.guesses.each do |guess|
			if guess.autoReplace and guess.autoAccept then
				self.testInfo.autoReplace = true
				self.testInfo.topGuess = guess.str
			end
		end

		# And now for the auto-correct if no auto-replace was returned
		if self.testInfo.topGuess.empty? then
			self.result.guesses.each do |guess|
				if not guess.autoReplace and guess.autoAccept then
					self.testInfo.autoAccept = true
					self.testInfo.topGuess = guess.str
				end
			end
		end

		# If no auto-correct or auto-replace then just go with the top guess.
		if self.testInfo.topGuess.empty? and not self.result.guesses.empty? then
			if self.result.guesses.length > 1 then
				self.testInfo.topGuess = self.result.guesses[1].str
			else
				self.testInfo.topGuess = self.result.guesses[0].str
			end
		end
	end
end

correct = 0
incorrect = 0
lineNo = 1

File.open("responses.csv") { |f|
	f.gets	# The first row is the column headers
	while line=f.gets do
		line.chomp!
		lineNo += 1
		next if line =~ /^#/
		items = line.split /,/
		if items.length == 2 then
			if items[0] == 'setLocale' then
				puts "Changing locale to #{items[1]}"
				$proxy.setLocale(items[1])
			end
		else
			info = TestInfo.new(line)
			if info.valid? then
				result = ResultInfo.new
				begin
					result.queryAndProccessResult(info.word)
					if result.testInfo == info then
						puts "correct: #{info.word} -> #{info.topGuess}"
						correct += 1
					else
						puts "Failure line #{lineNo}"
						puts "Expected: #{info}"
						puts "  Actual: #{result.testInfo}"
						incorrect += 1
					end
				rescue
					if info.topGuess == '<exception>' then
						puts "correct: #{info.word} -> #{info.topGuess}"
						correct += 1
					else
						puts "exception csv line #{lineNo}"
						incorrect += 1
					end
				end
			end
		end
	end	
}

puts "Correct: #{correct}, Incorrect: #{incorrect}"
exit incorrect
