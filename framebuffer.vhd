library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;

entity framebuffer is
port(
	-- Global signals
	clk : in std_logic;
	rst_n : in std_logic;
	
	-- VGA CTRL signals
	next_line : in std_logic;
	
	-- VRAM signals
	wr_addr : out std_logic_vector(7 downto 0);
	q : out std_logic_vector(15 downto 0);
	wren : out std_logic;
	
	--Avalon master signals
	address : out std_logic_vector(31 downto 0);
	read : out std_logic;
	flush : out std_logic;
	waitrequest : in std_logic;
	datavalid : in std_logic;
	readdata : in std_logic_vector(15 downto 0)
);
end entity;

architecture rtl of framebuffer is

signal wr_addr_r : std_logic_vector(wr_addr'range);
signal rd_addr_r : std_logic_vector(7 downto 0);
type t_stwrite is (WR_WAIT, WR_WRITE);
type t_stread is (RD_WAIT, RD_READ);
signal stwrite : t_stwrite;
signal stread : t_stread;

signal line : std_logic_vector(7 downto 0);
signal next_line_prec : std_logic;
signal ready_to_write : std_logic;

begin


XREG_READ : process(clk, rst_n)
begin
	if rst_n = '0' then
		stread <= RD_WAIT;
		rd_addr_r <= (others=>'0');
		line <= (others=>'1');
		next_line_prec <= '0';
		ready_to_write <= '0';
		flush <= '0';
		read <= '0';
	elsif clk'event and clk = '1' then
		next_line_prec <= next_line;
		if next_line = '1' and next_line_prec = '0' then
			rd_addr_r <= (others=>'0');
			stread <= RD_READ;
			ready_to_write <= '1';
			line <= line + 1;
			if unsigned(line) = 143 then
				line <= (others=>'1');
			end if;
			flush <= '1';
			read <= '1';
		elsif waitrequest = '0' then
			read <= '0';
			flush <= '0';
			ready_to_write <= '0';
			case stread is
			when RD_WAIT =>
			when RD_READ =>
				rd_addr_r <= rd_addr_r + 1;
				if unsigned(rd_addr_r) = 159 then
					stread <= RD_WAIT;
				end if;
				read <= '1';
				flush <= '0';
			end case;
		end if;
	end if;
end process;

X_RDADDR : process(line, rd_addr_r)
variable rd_addr : std_logic_vector(15 downto 0);
begin
	rd_addr := unsigned(line & "00000000") + unsigned(line & "000000") + unsigned(rd_addr_r & '0');
	address <= X"0100" & rd_addr;
end process;

XREG_WRITE : process(clk, rst_n)
begin
	if rst_n = '0' then
		stwrite <= WR_WAIT;
		wr_addr_r <= (others=>'1');
		q <= (others=>'0');
		wren <= '0';
	elsif clk'event and clk = '1' then
		q <= readdata;
		case stwrite is
		when WR_WAIT =>
			wren <= '0';
			wr_addr_r <= (others=>'1');
			if ready_to_write = '1' then
				stwrite <= WR_WRITE;
			else
				stwrite <= WR_WAIT;
			end if;
		when WR_WRITE =>
			if datavalid = '1' then
				wren <= '1';
				wr_addr_r <= wr_addr_r + 1;
			else
				wren <= '0';
			end if;
			if unsigned(wr_addr_r) = 159 then
				stwrite <= WR_WAIT;
			end if;
		end case;
	end if;
end process;

wr_addr <= wr_addr_r;

end architecture;