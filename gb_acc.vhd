LIBRARY ieee;
USE ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity spc_timer is
port(
	signal clk : in std_logic;
	signal reset : in std_logic;
	signal clk_en : in std_logic;
	signal start : in std_logic;
	signal done : out std_logic;
	signal dataa : in std_logic_vector(31 downto 0);
	signal datab : in std_logic_vector(31 downto 0);
	signal n : in std_logic_vector(1 downto 0); --0 = advance,1= get 2 = set
	signal result : out std_logic_vector(31 downto 0)
);
end entity;

architecture rtl of spc_timer is

	--Constants
	constant OP_ADVANCE std_logic_vector(1 downto 0) := "00";
	constant OP_GET     std_logic_vector(1 downto 0) := "01";
	constant OP_SET     std_logic_vector(1 downto 0) := "10";
	
	constant R_CPU_DIV	std_logic_vector(7 downto 0) := X"84";
	constant R_DIV	std_logic_vector(7 downto 0)     := X"04";
	constant R_CPU_TIM	std_logic_vector(7 downto 0) := X"86";
	constant R_TMA	std_logic_vector(7 downto 0)     := X"06";
	constant R_TIMA	std_logic_vector(7 downto 0)     := X"05";
	constant R_LCDC	std_logic_vector(7 downto 0)     := X"88";
	constant R_CPU_SND	std_logic_vector(7 downto 0) := X"82";

	--Registers
	
	signal	cpu_div		

begin

	XREG : process(clk,reset)
		variable clk_counter_next : integer range -4096 to 4095;
	begin
		if(reset='1') then
			for i in 0 to 2 loop
				timer_val_r(i) <= (others=>'0');
				timer_cnt_r(i) <= (others=>'0');
				timer_trg_r(i) <= (others=>'0');
				timer_en_r(i) <= '0';
			end loop;
			tick_divider_r <= "000";
			clk_counter_r <= 0;
		elsif(clk'event and clk='1') then
		
			for i in 0 to 2 loop
				if (timer_read(i)='1') then -- if a counter is read, it is reset
					timer_cnt_r(i) <= (others=>'0');
				end if;
				
				--Check is timer has been recently activated
				if(timer_activate(i)='1') then
					timer_val_r(i) <= (others=>'0');
					timer_cnt_r(i) <= (others=>'0');
				end if;
				
				--Checks for write
				if(n="1" and clk_en='1' and start='1') then
					if(dataa(15 downto 0) = X"00f1") then --timer control
						timer_en_r <= datab(2 downto 0);
					end if;
					if(dataa(15 downto 0) = X"00fa") then --trigger 0
						timer_trg_r(0) <= datab(7 downto 0);
					end if;
					if(dataa(15 downto 0) = X"00fb") then --trigger 1
						timer_trg_r(1) <= datab(7 downto 0);
					end if;
					if(dataa(15 downto 0) = X"00fc") then --trigger 2
						timer_trg_r(2) <= datab(7 downto 0);
					end if;
				end if;
			end loop;
				
			clk_counter_next:=clk_counter_r - clk_decrement;
			if(clk_counter_next<0) then
				for i in 0 to 2 loop
					
					--Checks if timer increments
					if((timer_en_r(i)='1') and ((i=2) or (tick_divider_r="111"))) then
						if(timer_val_next(i) >= timer_trg_r(i)) then
							timer_val_r(i) <= (others=>'0');
							if(timer_read(i)='1' or timer_activate(i)='1') then
								timer_cnt_r(i) <= "0001";
							else
								timer_cnt_r(i) <= unsigned(timer_cnt_r(i)) +1;
							end if;
						else
							timer_val_r(i) <= timer_val_next(i);
						end if;
					end if;
				end loop;
				
				tick_divider_r <= unsigned(tick_divider_r) + 1;
				clk_counter_r <= clk_counter_next +clk_load;
			else
				clk_counter_r <= clk_counter_next;
			end if;
		end if;
	end process;
	
	XALU : process(timer_val_r)
	begin
		for i in 0 to 2 loop
			timer_val_next(i) <= unsigned(timer_val_r(i)) + 1;
		end loop;
	end process;
	
	XREAD : process(clk_en,start,dataa,n,timer_cnt_r,timer_en_r,timer_val_r)
	begin
		timer_read <= "000";
		result <= (others=>'-');
		--result <= timer_en_r & "00000" & timer_val_r(0) & timer_val_r(1) & timer_val_r(2);
		if(clk_en='1' and start='1' and n="0") then
			if(dataa(15 downto 0)=X"00fd") then
				timer_read(0) <= '1';
				result <= "0000000000000000000000000000" & timer_cnt_r(0);
			end if;
			if(dataa(15 downto 0)=X"00fe") then
				timer_read(1) <= '1';
				result <= "0000000000000000000000000000" & timer_cnt_r(1);
			end if;
			if(dataa(15 downto 0)=X"00ff") then
				timer_read(2) <= '1';
				result <= "0000000000000000000000000000" & timer_cnt_r(2);
			end if;
		end if;
	end process;

	XACTIVATE : process(clk_en,start,dataa,datab,n,timer_en_r)
	begin
		timer_activate <= "000";
		if(clk_en='1' and start='1' and dataa(15 downto 0)=X"00f1" and n="1") then
			if(datab(0)='1' and timer_en_r(0)='0') then
				timer_activate(0) <= '1';
			end if;
			if(datab(1)='1' and timer_en_r(1)='0') then
				timer_activate(1) <= '1';
			end if;
			if(datab(2)='1' and timer_en_r(2)='0') then
				timer_activate(2) <= '1';
			end if;
		end if;
	end process;
	
	done <= start and clk_en;  --Single cycle operation

end architecture;